#include "convert_ir_to_asm.h"

#include <cstddef>
#include <ir_call_instr.h>
#include <ir_cast_instr.h>
#include <ir_cmp_instr.h>
#include <ir_ptr_instr.h>

#include "bkd_global.h"
#include "bkd_instr.h"
#include "bkd_module.h"
#include "bkd_reg.h"
#include "bkd_func.h"
#include "def.h"
#include "imm.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "type.h"
#include "value.h"
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace Backend {

[[noreturn]] void unreachable() {
    my_assert(false, "unreachable");
}

Module Convertor::convert(const Ir::pModule &mod)
{
    Module module;

    for (auto && global : mod->globs) {
        module.globs.push_back(convert(global));
    }

    for (auto && func : mod->funsDefined) {
        module.funcs.emplace_back(func);
        module.funcs.back().passes();
    }

    return module;
}

Global Convertor::convert(const Ir::pGlobal &glob)
{
    const auto& con = glob->con.v;
    auto name = glob->name().substr(1);
    switch (con.type()) {
    case VALUE_IMM:
        // all imm should be regarded as int32
        return {name, static_cast<int>(con.imm_value().val.ival % 2147483648)};
    case VALUE_ARRAY:
        return {name, con.array_value()};
    }
    unreachable();
}

void Func::translate()
{
    blocks.push_back(generate_prolog_tail());
    for (auto && block : ir_func->p) {
        blocks.push_back(translate(block));
    }
    blocks.emplace_back(name + "_epilog");
    blocks.front().body.push_back({ JInstr{ blocks[1].name } });
    std::vector<GReg> uses;
    if (is_float(type->ret_type)) {
        uses = {FReg::FA0};
    } else if (is_integer(type->ret_type)) {
        uses = {Reg::A0};
    }
    blocks.back().body.push_back(MachineInstr { ReturnInstr{ uses } });
}

Block Func::translate(const Ir::pBlock &block)
{
    Block bkd_block(ir_func->name() + "_" + block->label()->name());

    for (auto it = block->begin(); it != block->end(); ++it) {
        auto&& instr = *it;
        auto next = std::next(it);
        bool tail = false;
        auto res = translate(block, instr, next == block->end() ? nullptr : *next, tail);
        bkd_block.body.insert(bkd_block.body.end(),
            std::move_iterator(res.begin()), std::move_iterator(res.end()));
        if (tail) break;
    }

    return bkd_block;
}

struct ConvertBulk {
    MachineInstrs bulk;
    Func& func;
    explicit ConvertBulk(Func& func)
        : func(func) {}

    void add(MachineInstr const& value) {
        bulk.push_back(value);
    }

    Reg allocate_reg() {
        return (Reg) ~func.next_reg();
    }

    FReg allocate_freg() {
        return (FReg) ~func.next_reg();
    }

    Reg toReg(Ir::Val* val) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            // %0 => -1
            return (Reg) ~func.convert_reg(std::stoi(name.substr(1)));
        }
        auto tmp = allocate_reg();
        // if it is a immediate, load it
        add({ImmInstr {
            ImmInstrType::LI, tmp, std::stoi(name)
        } });
        return tmp;
    }

    FReg toFReg(Ir::Val* val) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            // %0 => -1
            return (FReg) ~func.convert_reg(std::stoi(name.substr(1)));
        }
        auto con = dynamic_cast<Ir::Const*>(val);
        assert(con->v.imm_value().ty == IMM_F32);
        float f = con->v.imm_value().val.f32val;
        auto ftmp = allocate_freg();
        auto tmp = allocate_reg();
        // // if it is a immediate, load it
        // // lLVM store float constant into hexadecimal integral representation of corresponding double value
        // union {
        //     unsigned long long ull;
        //     double d;
        // };
        // ull = std::stoull(name, nullptr, 16);
        // union {
        //     float f;
        //     int x;
        // };
        // f = d;
        add({ImmInstr {
            ImmInstrType::LI, tmp, *(int*)&f
        } });
        add({ FRegRegInstr {
            FRegRegInstrType::FMV_S_X, ftmp, tmp
        } });
        return ftmp;
    }

    RegRegInstrType selectRRType(Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return RegRegInstrType::ADDW;
            case Ir::INSTR_SUB:
                return RegRegInstrType::SUBW;
            case Ir::INSTR_MUL:
                return RegRegInstrType::MULW;
            case Ir::INSTR_SDIV:
            case Ir::INSTR_UDIV:
                return RegRegInstrType::DIVW;
            case Ir::INSTR_SREM:
            case Ir::INSTR_UREM:
                return RegRegInstrType::REMW;
            case Ir::INSTR_AND:
                return RegRegInstrType::AND;
            case Ir::INSTR_OR:
                return RegRegInstrType::OR;
            case Ir::INSTR_XOR:
                return RegRegInstrType::XOR;
            case Ir::INSTR_ASHR:
                return RegRegInstrType::SRAW;
            case Ir::INSTR_LSHR:
                return RegRegInstrType::SRLW;
            case Ir::INSTR_SHL:
                return RegRegInstrType::SLLW;
            default:
                unreachable();
        }
    }

    FRegFRegInstrType selectFRFRType(Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_FADD:
                return FRegFRegInstrType::FADD_S;
            case Ir::INSTR_FSUB:
                return FRegFRegInstrType::FSUB_S;
            case Ir::INSTR_FMUL:
                return FRegFRegInstrType::FMUL_S;
            case Ir::INSTR_FDIV:
                return FRegFRegInstrType::FDIV_S;
            default:
                unreachable();
        }
    }

    void convert_unary_instr(const Pointer<Ir::UnaryInstr> &instr) {
        auto rd = toFReg(instr.get());
        auto rs = toFReg(instr->operand(0)->usee);
        add({ FRegInstr {
                FRegInstrType::FNEG_S, rd, rs,
        } });
    }

    void convert_binary_instr(const Pointer<Ir::BinInstr> &instr) {
        if (is_float(instr->ty)) {
            auto rd = toFReg(instr.get());
            auto rs1 = toFReg(instr->operand(0)->usee);
            auto rs2 = toFReg(instr->operand(1)->usee);
            add({ FRegFRegInstr {
                selectFRFRType(instr->binType), rd, rs1, rs2,
            } });
        } else {
            auto rd = toReg(instr.get());
            auto rs1 = toReg(instr->operand(0)->usee);
            auto rs2 = toReg(instr->operand(1)->usee);
            add({ RegRegInstr {
                selectRRType(instr->binType), rd, rs1, rs2,
            } });
        }
    }

    void convert_return_instr(const Pointer<Ir::RetInstr> &instr) {
        auto ret = std::static_pointer_cast<Ir::RetInstr>(instr);
        if (ret->operand_size() > 0) {
            auto value = ret->operand(0)->usee;
            if (is_float(value->ty)) {
                auto fa0 = FReg::FA0;
                add({ FRegInstr{
                    FRegInstrType::FMV_S,
                    fa0,
                    toFReg(value)
                } });
            } else {
                auto a0 = Reg::A0;
                add({ RegInstr{
                    RegInstrType::MV,
                    a0,
                    toReg(value)
                } });
            }
        }
        add({ JInstr { func.name + "_epilog" } });
    }

    void convert_cast_instr(const Pointer<Ir::CastInstr> &instr) {
        switch (instr->method()) {
            case Ir::CAST_PTRTOINT:
            case Ir::CAST_INTTOPTR:
            case Ir::CAST_TRUNC:
            case Ir::CAST_FPEXT:
            case Ir::CAST_FPTRUNC:
                unreachable();
            case Ir::CAST_BITCAST: {
                auto rs = load_address(instr->operand(0)->usee);
                auto rd = toReg(instr.get());
                add({ RegInstr{
                    RegInstrType::MV, rd, rs
                } });
                break;
            }
            case Ir::CAST_SEXT:
            case Ir::CAST_ZEXT: {
                auto rs = toReg(instr->operand(0)->usee);
                auto rd = toReg(instr.get());
                add({ RegInstr{
                    RegInstrType::MV, rd, rs
                } });
                break;
            }
            case Ir::CAST_SITOFP:
            case Ir::CAST_UITOFP: {
                auto rs = toReg(instr->operand(0)->usee);
                auto rd = toFReg(instr.get());
                add({ FRegRegInstr{
                    FRegRegInstrType::FCVT_S_W, rd, rs
                } });
                break;
            }
            case Ir::CAST_FPTOSI:
            case Ir::CAST_FPTOUI: {
                auto rs = toFReg(instr->operand(0)->usee);
                auto rd = toReg(instr.get());
                add({ RegFRegInstr{
                    RegFRegInstrType::FCVT_W_S,
                    rd, rs
                } });
                break;
            }
        }
    }

    void convert_cmp_instr(bool& tail, const Ir::pInstr &terminator, const Pointer<Ir::CmpInstr> &instr) {
        auto ty = instr->operand(0)->usee->ty;
        if (is_float(ty)) {
            tail = false;
            auto rd = toReg(instr.get());
            auto rs1 = toFReg(instr->operand(0)->usee);
            auto rs2 = toFReg(instr->operand(1)->usee);
            switch (instr->cmp_type) {
                case Ir::CMP_OEQ:
                    add({ FCmpInstr {
                        FCmpInstrType::FEQ_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_OGT:
                    add({ FCmpInstr {
                        FCmpInstrType::FLT_S, rd, rs2, rs1,
                    } });
                    break;
                case Ir::CMP_OGE:
                    add({ FCmpInstr {
                        FCmpInstrType::FLE_S, rd, rs2, rs1,
                    } });
                    break;
                case Ir::CMP_OLT:
                    add({ FCmpInstr {
                        FCmpInstrType::FLT_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_OLE:
                    add({ FCmpInstr {
                        FCmpInstrType::FLE_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_UNE:
                    add({ FCmpInstr {
                        FCmpInstrType::FEQ_S, rd, rs1, rs2,
                    } });
                    add({ RegInstr {
                        RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                default:
                    unreachable();
            }
        } else {
            auto rd = toReg(instr.get());
            auto rs1 = toReg(instr->operand(0)->usee);
            auto rs2 = toReg(instr->operand(1)->usee);
            if (tail) {
                auto branch = dynamic_cast<Ir::BrCondInstr*>(terminator.get());
                auto label1 = func.name + "_" + branch->operand(1)->usee->name();
                auto label2 = func.name + "_" + branch->operand(2)->usee->name();
                switch (instr->cmp_type) {
                    case Ir::CMP_EQ:
                        add({ BranchInstr {
                            BranchInstrType::BNE, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    case Ir::CMP_NE:
                        add({ BranchInstr {
                            BranchInstrType::BEQ, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    case Ir::CMP_ULT:
                    case Ir::CMP_SLT:
                        add({ BranchInstr {
                            BranchInstrType::BGE, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    case Ir::CMP_UGE:
                    case Ir::CMP_SGE:
                        add({ BranchInstr {
                            BranchInstrType::BLT, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    case Ir::CMP_UGT:
                    case Ir::CMP_SGT:
                        add({ BranchInstr {
                              BranchInstrType::BLE, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    case Ir::CMP_ULE:
                    case Ir::CMP_SLE:
                        add({ BranchInstr {
                              BranchInstrType::BGT, rs1, rs2, label2
                        } });
                        add({ JInstr{ label1 } });
                        break;
                    default:
                        unreachable();
                }
            } else {
                switch (instr->cmp_type) {
                    case Ir::CMP_EQ:
                        add({ RegRegInstr {
                            RegRegInstrType::XOR, rd, rs1, rs2
                        } });
                        add({ RegInstr {
                            RegInstrType::SEQZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_NE:
                        add({ RegRegInstr {
                            RegRegInstrType::XOR, rd, rs1, rs2
                        } });
                        add({ RegInstr {
                            RegInstrType::SNEZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_ULT:
                    case Ir::CMP_SLT:
                        add({ RegRegInstr {
                            RegRegInstrType::SLT, rd, rs1, rs2
                        } });
                        break;
                    case Ir::CMP_UGE:
                    case Ir::CMP_SGE:
                        add({ RegRegInstr {
                            RegRegInstrType::SLT, rd, rs1, rs2
                        } });
                        add({ RegInstr {
                            RegInstrType::SEQZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_UGT:
                    case Ir::CMP_SGT:
                        add({ RegRegInstr {
                            RegRegInstrType::SLT, rd, rs2, rs1
                        } });
                        break;
                    case Ir::CMP_ULE:
                    case Ir::CMP_SLE:
                        add({ RegRegInstr {
                            RegRegInstrType::SLT, rd, rs2, rs1
                        } });
                        add({ RegInstr {
                            RegInstrType::SEQZ, rd, rd
                        } });
                        break;
                    default:
                        unreachable();
                }
            }
        }
    }

    void convert_branch_instr(const Pointer<Ir::BrCondInstr> &instr) {
        auto rs = toReg(instr->operand(0)->usee);
        auto label1 = func.name + "_" + instr->operand(1)->usee->name();
        auto label2 = func.name + "_" + instr->operand(2)->usee->name();
        // else branch: beqz rs label2, tend to be remote
        add({ RegLabelInstr{RegLabelInstrType::BEQZ, rs,  label2 } });
        // then branch: tend to follow current basic block
        add({ JInstr{ label1 } });
    }

    void convert_call_instr(bool& tail, const Pointer<Ir::CallInstr> &instr) {
        auto function_name = instr->operand(0)->usee->name();
        tail = tail && function_name == func.name;
        auto args_size = instr->operand_size() - 1;
        Reg arg = Reg::A0;
        FReg farg = FReg::FA0;
        std::vector<GReg> uses;
        std::vector<GReg> spilled;
        for (size_t i = 1; i <= args_size; ++i) {
            auto param = instr->operand(i)->usee;
            if (is_float(param->ty)) {
                auto rd = farg;
                auto rs = toFReg(param);
                if (rd <= FReg::FA7) {
                    add({  FRegInstr{
                        FRegInstrType::FMV_S, rd, rs
                    } });
                    uses.emplace_back(rd);
                    farg = (FReg)((int)farg + 1);
                } else {
                    spilled.emplace_back(rs);
                }
            } else {
                auto rd = arg;
                auto rs = toReg(param);
                if (rd <= Reg::A7) {
                    add({  RegInstr{
                        RegInstrType::MV, rd, rs
                    } });
                    uses.emplace_back(rd);
                    arg = (Reg)((int)arg + 1);
                } else {
                    spilled.emplace_back(rs);
                }
            }
        }
        func.frame.args = std::max(func.frame.args, spilled.size());
        for (size_t i = 0; i < spilled.size(); ++i) {
            auto rd = allocate_reg();
            auto rs = spilled[i];
            add({ LoadStackAddressInstr {rd, i,
                    tail ? LoadStackAddressInstr::Type::PARENT_ARG : LoadStackAddressInstr::Type::CHILD_ARG} });
            add({ StoreInstr {
                rs.index() ? LSType::FLOAT : LSType::DWORD, rs,  0, rd,
            } });
        }
        auto rt = instr->func_ty->ret_type;
        if (tail) function_name += "_prolog_tail";
        add({ CallInstr{ function_name, uses, tail } });
        if (tail) return;
        if (is_float(rt)) {
            auto rd = toFReg(instr.get());
            add({ FRegInstr{
                FRegInstrType::FMV_S, rd, FReg::FA0
            } });
        } else if (is_integer(rt)) {
            auto rd = toReg(instr.get());
            add({ RegInstr{
                RegInstrType::MV, rd, Reg::A0
            } });
        }
    }

    Reg load_address(Ir::Val* val) {
        auto name = val->name();
        if (name[0] == '@') {
            // global
            auto rd = allocate_reg();
            add({ LoadAddressInstr{
               rd, name.substr(1)
            } });
            return rd;
        }
        if (auto alloc = dynamic_cast<Ir::AllocInstr*>(val)) {
            // local
            auto index = func.localIndex[alloc];
            auto rd = allocate_reg();
            add({  LoadStackAddressInstr{
                rd, index
            } });
            return rd;
        }
        // gep
        return toReg(val);
    }

    void convert_store_instr(const Pointer<Ir::StoreInstr> &instr) {
        auto address = load_address(instr->operand(0)->usee);
        auto value = instr->operand(1)->usee;
        if (is_float(value->ty)) {
            auto rs = toFReg(value);
            add({ StoreInstr{
                LSType::FLOAT, rs, 0, address
            } });
        } else {
            bool wide = value->ty->length() > 4;
            auto rs = toReg(value);
            add({ StoreInstr{
                wide ? LSType::DWORD : LSType::WORD, rs, 0, address
            } });
        }
    }

    void convert_load_instr(const Pointer<Ir::LoadInstr> &instr) {
        auto address = load_address(instr->operand(0)->usee);
        if (is_float(instr->ty)) {
            auto rd = toFReg(instr.get());
            add({ LoadInstr{
                LSType::FLOAT, rd, 0, address
            } });
        } else {
            bool wide = instr->ty->length() > 4;
            auto rd = toReg(instr.get());
            add({ LoadInstr{
                wide ? LSType::DWORD : LSType::WORD, rd, 0, address
            } });
        }
    }

    void convert_gep_instr(const Pointer<Ir::ItemInstr> &instr) {
        auto base = instr->operand(0)->usee;
        auto type = base->ty;
        type = to_pointed_type(type);
        if (!instr->get_from_local) {
            type = make_array_type(type, 0);
        }
        auto rs = load_address(base);
        auto rd = toReg(instr.get());
        for (size_t dim = 1; dim < instr->operand_size(); ++dim) {
            type = to_elem_type(type);
            int step = type->length();
            auto index = instr->operand(dim)->usee;
            auto r1 = allocate_reg();
            auto r2 = allocate_reg();
            auto r3 = allocate_reg();
            add({ImmInstr {
                ImmInstrType::LI, r1, step
            } });
            add({ RegRegInstr {
                RegRegInstrType::MUL, r2, r1, toReg(index)
            } });
            add({ RegRegInstr {
                RegRegInstrType::ADD, r3, rs, r2
            } });
            rs = r3;
        }
        add({  RegInstr{
            RegInstrType::MV, rd, rs
        } });
    }

    void convert_alloca_instr(const Pointer<Ir::AllocInstr> &instr) {
        auto type = to_pointed_type(instr->ty);
        int size = type->length();
        if (size == 1) size = 4; // for bool
        assert(size % 4 == 0);
        auto index = func.frame.push(size);
        func.localIndex[instr.get()] = index;
    }

};

MachineInstrs Func::translate(const Ir::pBlock &block, const Ir::pInstr &instr, const Ir::pInstr &next, bool& tail)
{
    ConvertBulk bulk(*this);
    switch (instr->instr_type()) {
        case Ir::INSTR_RET:
            bulk.convert_return_instr(std::dynamic_pointer_cast<Ir::RetInstr>(instr));
            tail = true;
            break;
        case Ir::INSTR_BR:
            bulk.add({ JInstr{ name + "_" + instr->operand(0)->usee->name() } });
            tail = true;
            break;
        case Ir::INSTR_BR_COND:
            bulk.convert_branch_instr(std::static_pointer_cast<Ir::BrCondInstr>(instr));
            tail = true;
            break;
        case Ir::INSTR_CALL: {
            auto terminator = *block->rbegin();
            tail = terminator->instr_type() == Ir::INSTR_RET && next == terminator
                    && instr->users.size() == 1 && instr->users[0]->user == terminator.get();
            bulk.convert_call_instr(tail, std::static_pointer_cast<Ir::CallInstr>(instr));
            break;
        }
        case Ir::INSTR_UNARY:
            bulk.convert_unary_instr(std::static_pointer_cast<Ir::UnaryInstr>(instr));
            break;
        case Ir::INSTR_BINARY:
            bulk.convert_binary_instr(std::static_pointer_cast<Ir::BinInstr>(instr));
            break;
        case Ir::INSTR_CAST:
            bulk.convert_cast_instr(std::static_pointer_cast<Ir::CastInstr>(instr));
            break;
        case Ir::INSTR_CMP: {
            auto terminator = *block->rbegin();
            tail = terminator->instr_type() == Ir::INSTR_BR_COND && next == terminator
                    && instr->users.size() == 1 && instr->users[0]->user == terminator.get();
            bulk.convert_cmp_instr(tail, terminator, std::static_pointer_cast<Ir::CmpInstr>(instr));
            break;
        }
        case Ir::INSTR_STORE:
            bulk.convert_store_instr(std::static_pointer_cast<Ir::StoreInstr>(instr));
            break;
        case Ir::INSTR_LOAD:
            bulk.convert_load_instr(std::static_pointer_cast<Ir::LoadInstr>(instr));
            break;
        case Ir::INSTR_ITEM:
            bulk.convert_gep_instr(std::static_pointer_cast<Ir::ItemInstr>(instr));
            break;
        case Ir::INSTR_ALLOCA:
            bulk.convert_alloca_instr(std::static_pointer_cast<Ir::AllocInstr>(instr));
            break;
        case Ir::INSTR_FUNC:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_PHI:
        case Ir::INSTR_SYM:
        case Ir::INSTR_UNREACHABLE:
            // just ignore all these ir
            break;
    }
    return bulk.bulk;
}

std::vector<MachineInstr> spaddi(Reg rd, int imm) {
    if (check_itype_immediate(imm)) {
        return {{ RegImmInstr{ RegImmInstrType::ADDI, rd, Reg::SP, imm} }};
    } else {
        return {
            { ImmInstr { ImmInstrType::LI, Reg::T0, imm } },
            { RegRegInstr{ RegRegInstrType::ADD, rd, Reg::SP, Reg::T0} }
        };
    }
}

Block Func::generate_prolog_tail() {
    Block block(name + "_prolog_tail");
    auto add = [&](MachineInstr const& value) {
        block.body.push_back(value);
    };
    Reg arg = Reg::A0;
    FReg farg = FReg::FA0;
    auto next_arg = [this, add, i = (size_t)0] () mutable {
        auto rd = (Reg) ~next_reg();
        add({ LoadStackAddressInstr {rd, i++, LoadStackAddressInstr::Type::PARENT_ARG} });
        return rd;
    };
    int reg = 0;
    for (auto&& at : type->arg_type) {
        if (is_float(at)) {
            auto rd = (FReg) ~reg++;
            auto rs = farg;
            if (rs <= FReg::FA7) {
                add({  FRegInstr{
                    FRegInstrType::FMV_S, rd, rs
                } });
                farg = (FReg)((int)farg + 1);
            } else {
                add({ LoadInstr {
                    LSType::FLOAT, rd,  0, next_arg(),
                } });
            }
        } else {
            auto rd = (Reg) ~reg++;
            auto rs = arg;
            if (rs <= Reg::A7) {
                add({  RegInstr{
                    RegInstrType::MV, rd, rs
                } });
                arg = (Reg)((int)arg + 1);
            } else {
                add({ LoadInstr {
                    LSType::DWORD, rd,  0, next_arg(),
                } });
            }
        }
    }
    return block;
}

void Func::generate_prolog() {
    int frame_size = frame.size();
    Block block(name + "_prolog");
    std::vector<MachineInstr> prolog = spaddi(Reg::SP, -frame_size);
    auto save = [&prolog](int offset, GReg reg) {
        auto type = reg.index() ? LSType::FLOAT : LSType::DWORD;
        if (check_itype_immediate(offset)) {
            prolog.push_back({ StoreInstr {
                type, reg,  offset, Reg::SP,
            } });
        } else {
            prolog.push_back({ ImmInstr {
                ImmInstrType::LI, Reg::T0, offset
            } });
            prolog.push_back({ RegRegInstr {
                RegRegInstrType::ADD, Reg::T0, Reg::T0, Reg::SP,
            } });
            prolog.push_back({ StoreInstr {
                type, reg, 0, Reg::T0,
            } });
        }
    };
    save(frame_size - 8, Reg::RA);
    for (auto& [reg, index] : saved_registers) {
        save(frame_size - frame.locals[index].offset, reg);
    }
    block.body.assign(prolog.begin(), prolog.end());
    auto& prolog_tail = blocks.front();
    blocks.push_front(std::move(block));
    blocks.front().out_blocks.push_back(&prolog_tail);
    prolog_tail.in_blocks.push_back(&blocks.front());
}

void Func::remove_pseudo() {
    int frame_size = frame.size();
    for (auto&& block : blocks) {
        for (auto it = block.body.begin(); it != block.body.end(); ) {
            auto instr = *it;
            if (instr.instr_type() == MachineInstr::Type::LOAD_STACK_ADDRESS) {
                auto LSA = instr.as<LoadStackAddressInstr>();
                int offset = 0;
                switch (LSA.type) {
                    case LoadStackAddressInstr::Type::LOCAL:
                        offset = frame_size - frame.locals[LSA.index].offset;
                        break;
                    case LoadStackAddressInstr::Type::PARENT_ARG:
                        offset = frame_size + (int)LSA.index * 8;
                        break;
                    case LoadStackAddressInstr::Type::CHILD_ARG:
                        offset = (int)LSA.index * 8;
                        break;
                }

                auto replacement = spaddi(LSA.rd, offset);
                it = block.body.erase(it);
                it = block.body.insert(it, replacement.begin(), replacement.end());
            } else {
                ++it;
            }
        }
    }
}

void Func::generate_epilog() {
    int frame_size = frame.size();
    std::vector<MachineInstr> prepend;
    auto reload = [&prepend](int offset, GReg reg){
        auto type = reg.index() ? LSType::FLOAT : LSType::DWORD;
        if (check_itype_immediate(offset)) {
            prepend.push_back({ LoadInstr {
                type, reg, offset, Reg::SP,
            } });
        } else {
            prepend.push_back({ ImmInstr {
                ImmInstrType::LI, Reg::T0, offset
            } });
            prepend.push_back({ RegRegInstr {
                RegRegInstrType::ADD, Reg::T0, Reg::T0, Reg::SP,
            } });
            prepend.push_back({ LoadInstr {
                type, reg, 0, Reg::T0,
            } });
        }
    };
    for (auto&& [reg, index] : saved_registers) {
        reload(frame_size - frame.locals[index].offset, reg);
    }
    reload(frame_size - 8, Reg::RA);
    {
        auto rewind = spaddi(Reg::SP, frame_size);
        prepend.insert(prepend.end(), rewind.begin(), rewind.end());
    }
    blocks.back().body.insert(blocks.back().body.begin(), prepend.begin(), prepend.end());

}


} // namespace BackendConvertor

