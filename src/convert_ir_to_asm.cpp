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
    blocks.emplace_back(name + "_prolog");
    for (auto && block : ir_func->p) {
        blocks.push_back(translate(block));
    }
    blocks.emplace_back(name + "_epilog");
}

Block Func::translate(const Ir::pBlock &block)
{
    Block bkd_block(ir_func->name() + "_" + block->label()->name());

    for (auto && instr : *block) {
        auto res = translate(instr);
        bkd_block.body.insert(bkd_block.body.end(),
            std::move_iterator(res.begin()), std::move_iterator(res.end()));
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

    template<typename T>
    T selector(ImmType ty,
        T i32_use,
        T i64_use) {
        switch (ty) {
            case IMM_I1:
            case IMM_I8:
            case IMM_I16:
            case IMM_I32:
            case IMM_U1:
            case IMM_U8:
            case IMM_U16:
            case IMM_U32:
                return i32_use;
            case IMM_I64:
            case IMM_U64:
                return i64_use;
            default:
                unreachable();
        }
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
        auto ftmp = allocate_freg();
        auto tmp = allocate_reg();
        // if it is a immediate, load it
        // lLVM store float constant into hexadecimal integral representation of corresponding double value
        union {
            unsigned long long ull;
            double d;
        };
        ull = std::stoull(name, nullptr, 16);
        union {
            float f;
            int x;
        };
        f = d;
        add({ImmInstr {
            ImmInstrType::LI, tmp, x
        } });
        add({ FRegRegInstr {
            FRegRegInstrType::FMV_S_X, ftmp, tmp
        } });
        return ftmp;
    }

    RegRegInstrType selectRRType(ImmType ty, Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return selector(ty,
                    RegRegInstrType::ADD,
                    RegRegInstrType::ADDW);
            case Ir::INSTR_SUB:
                return selector(ty,
                    RegRegInstrType::SUB,
                    RegRegInstrType::SUBW);
            case Ir::INSTR_MUL:
                return selector(ty,
                    RegRegInstrType::MUL,
                    RegRegInstrType::MULW);
            case Ir::INSTR_SDIV:
            case Ir::INSTR_UDIV:
                return selector(ty,
                    RegRegInstrType::DIV,
                    RegRegInstrType::DIVW);
            case Ir::INSTR_SREM:
            case Ir::INSTR_UREM:
                return selector(ty,
                    RegRegInstrType::REM,
                    RegRegInstrType::REMW);
            case Ir::INSTR_AND:
                return RegRegInstrType::AND;
            case Ir::INSTR_OR:
                return RegRegInstrType::OR;
            case Ir::INSTR_ASHR:
                return selector(ty,
                    RegRegInstrType::SRA,
                    RegRegInstrType::SRAW);
                case Ir::INSTR_LSHR:
                return selector(ty,
                    RegRegInstrType::SRL,
                    RegRegInstrType::SRLW);
            case Ir::INSTR_SHL:
                return selector(ty,
                    RegRegInstrType::SLL,
                    RegRegInstrType::SLLW);
            default:
                unreachable();
        }
    }

    FRegFRegInstrType selectFRFRType(ImmType ty, Ir::BinInstrType bin) {
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
                selectFRFRType(to_basic_type(instr->ty)->ty, instr->binType), rd, rs1, rs2,
            } });
        } else {
            auto rd = toReg(instr.get());
            auto rs1 = toReg(instr->operand(0)->usee);
            auto rs2 = toReg(instr->operand(1)->usee);
            add({ RegRegInstr {
                selectRRType(to_basic_type(instr->ty)->ty, instr->binType), rd, rs1, rs2,
            } });
        }
    }

    void convert_return_instr(const Pointer<Ir::RetInstr> &instr) {
        auto ret = std::static_pointer_cast<Ir::RetInstr>(instr);
        if (ret->operand_size() > 0) {
            if (is_float(instr->ty)) {
                auto fa0 = FReg::FA0;
                add({ FRegInstr{
                    FRegInstrType::FMV_S,
                    fa0,
                    toFReg(ret->operand(0)->usee)
                } });
            } else {
                auto a0 = Reg::A0;
                add({ RegInstr{
                    RegInstrType::MV,
                    a0,
                    toReg(ret->operand(0)->usee)
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
            case Ir::CAST_BITCAST:
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

    void convert_cmp_instr(const Pointer<Ir::CmpInstr> &instr) {
        auto ty = instr->operand(0)->usee->ty;
        if (is_float(ty)) {
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
            switch (instr->cmp_type) {
                case Ir::CMP_EQ:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                case Ir::CMP_NE:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SNEZ, rd, rd
                    } });
                    break;
                case Ir::CMP_ULT:
                case Ir::CMP_SLT:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SLTZ, rd, rd
                    } });
                    break;
                case Ir::CMP_UGE:
                case Ir::CMP_SGE:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SLTZ, rd, rd
                    } });
                    add({ RegInstr {
                        RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                case Ir::CMP_UGT:
                case Ir::CMP_SGT:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SGTZ, rd, rd
                    } });
                    break;
                case Ir::CMP_ULE:
                case Ir::CMP_SLE:
                    add({ RegRegInstr {
                        RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ RegInstr {
                        RegInstrType::SGTZ, rd, rd
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

    void convert_branch_instr(const Pointer<Ir::BrCondInstr> &instr) {
        auto rs = toReg(instr->operand(0)->usee);
        auto label1 = func.name + "_" + instr->operand(1)->usee->name();
        auto label2 = func.name + "_" + instr->operand(2)->usee->name();
        // else branch: beqz rs label2, tend to be remote
        add({ RegLabelInstr{RegLabelInstrType::BEQZ, rs,  label2 } });
        // then branch: tend to follow current basic block
        add({ JInstr{ label1 } });
    }

    void convert_call_instr(const Pointer<Ir::CallInstr> &instr) {
        Reg arg = Reg::A0;
        FReg farg = FReg::FA0;
        func.frame.at_least_args(instr->operand_size() - 1);
        auto next_arg = [this, i = (size_t)0] () mutable {
            auto rd = allocate_reg();
            add({ LoadStackAddressInstr {rd, i++, true} });
            return rd;
        };
        for (size_t i = 1; i < instr->operand_size(); ++i) {
            auto param = instr->operand(i)->usee;
            if (is_float(param->ty)) {
                auto rd = farg;
                auto rs = toFReg(param);
                if (rd <= FReg::FA7) {
                    add({  FRegInstr{
                        FRegInstrType::FMV_S, rd, rs
                    } });
                    farg = (FReg)((int)farg + 1);
                } else {
                    add({ StoreInstr {
                        LSType::FLOAT, rs,  0, next_arg(),
                    } });
                }
            } else {
                auto rd = arg;
                auto rs = toReg(param);
                if (rd <= Reg::A7) {
                    add({  RegInstr{
                        RegInstrType::MV, rd, rs
                    } });
                    arg = (Reg)((int)arg + 1);
                } else {
                    add({ StoreInstr {
                        LSType::DWORD, rs,  0, next_arg(),
                    } });
                }
            }
        }
        add({ CallInstr{instr->operand(0)->usee->name()} });
        auto rt = instr->func_ty->ret_type;
        if (is_float(rt)) {
            auto rd = toFReg(instr.get());
            auto fa0 = FReg::FA0;
            add({ FRegInstr{
                FRegInstrType::FMV_S, rd, fa0
            } });
        } else if (is_integer(rt)) {
            auto rd = toReg(instr.get());
            auto a0 = Reg::A0;
            add({ RegInstr{
                RegInstrType::MV, rd, a0
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
        return toReg(val);
    }

    void convert_store_instr(const Pointer<Ir::StoreInstr> &instr) {
        auto address = load_address(instr->operand(0)->usee);
        if (is_float(instr->ty)) {
            auto rs = toFReg(instr->operand(1)->usee);
            add({ StoreInstr{
                LSType::FLOAT, rs, 0, address
            } });
        } else {
            auto rs = toReg(instr->operand(1)->usee);
            add({ StoreInstr{
                LSType::WORD, rs, 0, address
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
            auto rd = toReg(instr.get());
            add({ LoadInstr{
                LSType::WORD, rd, 0, address
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
            auto index = instr->operand(dim++)->usee;
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
        auto rd = toReg(instr.get());
        add({  LoadStackAddressInstr{
            rd, index
        } });
    }

};

MachineInstrs Func::translate(const Ir::pInstr &instr)
{
    ConvertBulk bulk(*this);
    switch (instr->instr_type()) {
        case Ir::INSTR_RET:
            bulk.convert_return_instr(std::dynamic_pointer_cast<Ir::RetInstr>(instr));
            break;
        case Ir::INSTR_BR:
            bulk.add({ JInstr{ name + "_" + instr->operand(0)->usee->name() } });
            break;
        case Ir::INSTR_BR_COND:
            bulk.convert_branch_instr(std::static_pointer_cast<Ir::BrCondInstr>(instr));
            break;
        case Ir::INSTR_CALL:
            bulk.convert_call_instr(std::static_pointer_cast<Ir::CallInstr>(instr));
            break;
        case Ir::INSTR_UNARY:
            bulk.convert_unary_instr(std::static_pointer_cast<Ir::UnaryInstr>(instr));
            break;
        case Ir::INSTR_BINARY:
            bulk.convert_binary_instr(std::static_pointer_cast<Ir::BinInstr>(instr));
            break;
        case Ir::INSTR_CAST:
            bulk.convert_cast_instr(std::static_pointer_cast<Ir::CastInstr>(instr));
            break;
        case Ir::INSTR_CMP:
            bulk.convert_cmp_instr(std::static_pointer_cast<Ir::CmpInstr>(instr));
            break;
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

bool Func::peephole() {
    return false;
}


void Func::generate_prolog() {
    auto add = [this](MachineInstr const& value) {
        blocks.front().body.push_back(value);
    };
    {
        int sp = (int)frame.size();
        add({ RegImmInstr {
            RegImmInstrType::ADDI, Reg::SP, Reg::SP, -sp
        } });
        add({ StoreInstr {
            LSType::DWORD, Reg::RA,  sp - 8, Reg::SP,
        } });
    }
    Reg arg = Reg::A0;
    FReg farg = FReg::FA0;
    auto next_arg = [this, add, i = (size_t)0] () mutable {
        auto rd = (Reg) ~next_reg();
        add({ LoadStackAddressInstr {rd, i++, true} });
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
}

void Func::generate_epilog() {
    auto add = [this](MachineInstr const& value) {
        blocks.back().body.push_back(value);
    };
    {
        int sp = (int)frame.size();
        add({ LoadInstr {
            LSType::DWORD, Reg::RA,  sp - 8, Reg::SP,
        } });
        add({ RegImmInstr {
            RegImmInstrType::ADDI, Reg::SP, Reg::SP, sp
        } });
        add({ ReturnInstr{} });
    }
}


} // namespace BackendConvertor

