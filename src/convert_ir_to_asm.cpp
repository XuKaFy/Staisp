#include "convert_ir_to_asm.h"

#include <cstddef>
#include <ir_call_instr.h>
#include <ir_cast_instr.h>
#include <ir_cmp_instr.h>
#include <ir_phi_instr.h>
#include <ir_ptr_instr.h>

#include "bkd_global.h"
#include "bkd_instr.h"
#include "bkd_module.h"
#include "bkd_reg.h"
#include "bkd_func.h"
#include "def.h"
#include "imm.h"
#include "ir_func_defined.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "ir_val.h"
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

void remove_unused_initialization(const Ir::pModule &mod)
{
    Ir::pFuncDefined unused;
    for (const auto &i : mod->funsDefined) {
        if (!(*i).ast_root) { // __buildin_initialization
            unused = i;
            break;
        }
    }
    if (!unused) return ;
    for (const auto &i : unused->users()) {
        my_assert(i->user->type() == Ir::VAL_INSTR, "?");
        auto instr = dynamic_cast<Ir::Instr*>(i->user);
        my_assert(instr->instr_type(), Ir::INSTR_CALL);
        instr->block()->erase(instr);
        break;
    }
    mod->remove_unused_function();
}

Module Convertor::convert(const Ir::pModule &mod)
{
    remove_unused_initialization(mod);
    mod->remove_unused_function();

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
        block_map[block.get()] = &blocks.back();
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
        auto res = translate(instr, next == block->end() ? nullptr : *next, tail);
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

    static std::optional<int> to_itype_immediate(Ir::Val* val, bool negative = false) {
        auto name = val->name();
        if (name[0] != '%' && name[0] != '@') {
            auto imm = std::stoi(name);
            if (negative) imm = -imm;
            if (check_itype_immediate(imm))
                return imm;
        }
        return std::nullopt;
    }

    Reg li(int imm) {
        if (imm == 0) return Reg::ZERO;
        auto tmp = allocate_reg();
        // if it is a immediate, load it
        add({ImmInstr {
            ImmInstrType::LI, tmp, imm
        } });
        return tmp;
    }

    Reg toReg(Ir::Val* val) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            // %0 => -1
            return (Reg) ~func.convert_reg(std::stoi(name.substr(1)));
        }
        auto imm = std::stoi(name);
        return li(imm);
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
        int imm = *(int*)&f;
        auto tmp = allocate_freg();
        add({ FRegRegInstr {
            FRegRegInstrType::FMV_S_X, tmp, li(imm)
        } });
        return tmp;
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
            case Ir::INSTR_SLT:
                return RegRegInstrType::SLT;
            default:
                unreachable();
        }
    }

    std::optional<RegImmInstrType> selectRIType(Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return RegImmInstrType::ADDIW;
            case Ir::INSTR_AND:
                return RegImmInstrType::ANDI;
            case Ir::INSTR_OR:
                return RegImmInstrType::ORI;
            case Ir::INSTR_XOR:
                return RegImmInstrType::XORI;
            case Ir::INSTR_ASHR:
                return RegImmInstrType::SRAIW;
            case Ir::INSTR_LSHR:
                return RegImmInstrType::SRLIW;
            case Ir::INSTR_SHL:
                return RegImmInstrType::SLLIW;
            case Ir::INSTR_SLT:
                return RegImmInstrType::SLTI;
        }
        return std::nullopt;
    }

    std::optional<RegImmInstrType> selectIRType(Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return RegImmInstrType::ADDIW;
            case Ir::INSTR_AND:
                return RegImmInstrType::ANDI;
            case Ir::INSTR_OR:
                return RegImmInstrType::ORI;
            case Ir::INSTR_XOR:
                return RegImmInstrType::XORI;
        }
        return std::nullopt;
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

    void add_int_binary(Reg rd, Ir::BinInstrType type, Ir::Val* lhs, Ir::Val* rhs) {
        if (auto imm = to_itype_immediate(rhs)) {
            if (auto ritype = selectRIType(type)) {
                add({ RegImmInstr {
                    ritype.value(), rd, toReg(lhs), imm.value(),
                } });
                return;
            }
        }
        if (auto imm = to_itype_immediate(rhs, true)) {
            if (type == Ir::INSTR_SUB) {
                add({ RegImmInstr {
                    RegImmInstrType::ADDIW, rd, toReg(lhs), imm.value(),
                } });
                return;
            }
        }
        if (auto imm = to_itype_immediate(lhs)) {
            if (auto ritype = selectIRType(type)) {
                add({ RegImmInstr {
                    ritype.value(), rd, toReg(rhs), imm.value(),
                } });
                return;
            }
        }
        auto rs1 = toReg(lhs);
        auto rs2 = toReg(rhs);
        add({ RegRegInstr {
            selectRRType(type), rd, rs1, rs2,
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
            auto lhs = instr->operand(0)->usee;
            auto rhs = instr->operand(1)->usee;
            add_int_binary(rd, instr->binType, lhs, rhs);
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
            auto lhs = instr->operand(0)->usee;
            auto rhs = instr->operand(1)->usee;
            if (tail) {
                auto branch = dynamic_cast<Ir::BrCondInstr*>(terminator.get());
                auto rs1 = toReg(lhs);
                auto rs2 = toReg(rhs);
                auto label1 = branch->operand(1)->usee;
                auto label2 = branch->operand(2)->usee;
                submit_phi(label1, instr->block());
                submit_phi(label2, instr->block());
                auto label1name = func.name + "_" + label1->name();
                auto label2name = func.name + "_" + label2->name();
                switch (instr->cmp_type) {
                    case Ir::CMP_EQ:
                        add({ BranchInstr {
                            BranchInstrType::BNE, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    case Ir::CMP_NE:
                        add({ BranchInstr {
                            BranchInstrType::BEQ, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    case Ir::CMP_ULT:
                    case Ir::CMP_SLT:
                        add({ BranchInstr {
                            BranchInstrType::BGE, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    case Ir::CMP_UGE:
                    case Ir::CMP_SGE:
                        add({ BranchInstr {
                            BranchInstrType::BLT, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    case Ir::CMP_UGT:
                    case Ir::CMP_SGT:
                        add({ BranchInstr {
                              BranchInstrType::BLE, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    case Ir::CMP_ULE:
                    case Ir::CMP_SLE:
                        add({ BranchInstr {
                              BranchInstrType::BGT, rs1, rs2, label2name
                        } });
                        add({ JInstr{ label1name } });
                        break;
                    default:
                        unreachable();
                }
            } else {
                switch (instr->cmp_type) {
                    case Ir::CMP_EQ:
                        add_int_binary(rd, Ir::INSTR_XOR, lhs, rhs);
                        add({ RegInstr {
                            RegInstrType::SEQZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_NE:
                        add_int_binary(rd, Ir::INSTR_XOR, lhs, rhs);
                        add({ RegInstr {
                            RegInstrType::SNEZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_ULT:
                    case Ir::CMP_SLT:
                        add_int_binary(rd, Ir::INSTR_SLT, lhs, rhs);
                        break;
                    case Ir::CMP_UGE:
                    case Ir::CMP_SGE:
                        add_int_binary(rd, Ir::INSTR_SLT, lhs, rhs);
                        add({ RegInstr {
                            RegInstrType::SEQZ, rd, rd
                        } });
                        break;
                    case Ir::CMP_UGT:
                    case Ir::CMP_SGT:
                        add_int_binary(rd, Ir::INSTR_SLT, rhs, lhs);
                        break;
                    case Ir::CMP_ULE:
                    case Ir::CMP_SLE:
                        add_int_binary(rd, Ir::INSTR_SLT, rhs, lhs);
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
        auto label1 = instr->operand(1)->usee;
        auto label2 = instr->operand(2)->usee;
        submit_phi(label1, instr->block());
        submit_phi(label2, instr->block());
        auto label1name = func.name + "_" + label1->name();
        auto label2name = func.name + "_" + label2->name();
        // else branch: beqz rs label2, tend to be remote
        add({ BranchInstr{BranchInstrType::BEQ, rs, Reg::ZERO, label2name } });
        // then branch: tend to follow current basic block
        add({ JInstr{ label1name } });
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
            auto rs = spilled[i];
            add({ StoreStackInstr {
                rs.index() ? LSType::FLOAT : LSType::DWORD, rs, i,
                tail ? StackObjectType::PARENT_ARG : StackObjectType::CHILD_ARG
            }});
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

    Reg load_address(Ir::Val* address) {
        auto name = address->name();
        if (name[0] == '@') {
            // global
            auto rd = allocate_reg();
            add({ LoadAddressInstr{
               rd, name.substr(1)
            } });
            return rd;
        }
        if (auto alloc = dynamic_cast<Ir::AllocInstr*>(address)) {
            // local
            auto index = func.localIndex[alloc];
            if (func.localAddress.count(index)) return func.localAddress[index];
            auto rd = allocate_reg();
            add({  LoadStackAddressInstr{
                rd, index
            } });
            return func.localAddress[index] = rd;
        }
        // gep
        return toReg(address);
    }

    static LSType getLSType(Ir::Val* val) {
        return is_float(val->ty) ? LSType::FLOAT : val->ty->length() > 4 ? LSType::DWORD : LSType::WORD;
    }

    GReg to_generic_reg(Ir::Val* val) {
        if (is_float(val->ty)) return toFReg(val); else return toReg(val);
    }

    void convert_store_instr(const Pointer<Ir::StoreInstr> &instr) {
        auto from = instr->operand(1)->usee;
        auto address = instr->operand(0)->usee;
        auto name = address->name();
        auto type = getLSType(from);
        auto rs = to_generic_reg(from);
        if (name[0] == '@') {
            // global
            add({ StoreGlobalInstr{
               type, rs, name.substr(1)
            } });
            return;
        }
        if (auto alloc = dynamic_cast<Ir::AllocInstr*>(address)) {
            // local
            auto index = func.localIndex[alloc];
            add({  StoreStackInstr{
                type, rs, index
            } });
            return;
        }
        if (auto gep = dynamic_cast<Ir::MiniGepInstr*>(address);
                gep && gep->users().size() == 1 && gep->users()[0]->user == instr.get()) {
            auto [reg, offset] = parse_mini_gep(gep);
            add({ StoreInstr{
                type, rs, offset, reg
            } });
            return;
        }
        add({ StoreInstr{
            type, rs, 0, toReg(address)
        } });
    }

    void convert_load_instr(const Pointer<Ir::LoadInstr> &instr) {
        auto to = instr.get();
        auto address = instr->operand(0)->usee;
        auto name = address->name();
        auto type = getLSType(to);
        auto rd = to_generic_reg(to);
        if (name[0] == '@') {
            // global
            add({ LoadGlobalInstr{
               type, rd, name.substr(1)
            } });
            return;
        }
        if (auto alloc = dynamic_cast<Ir::AllocInstr*>(address)) {
            // local
            auto index = func.localIndex[alloc];
            add({  LoadStackInstr{
                type, rd, index
            } });
            return;
        }
        if (auto gep = dynamic_cast<Ir::MiniGepInstr*>(address);
                gep && gep->users().size() == 1 && gep->users()[0]->user == instr.get()) {
            auto [reg, offset] = parse_mini_gep(gep);
            add({ LoadInstr{
                type, rd, offset, reg
            } });
            return;
        }
        add({ LoadInstr{
            type, rd, 0, toReg(address)
        } });
    }

#ifdef USING_MINI_GEP
    std::pair<Reg, int> parse_mini_gep(Ir::MiniGepInstr* instr) {
        auto base = instr->operand(0)->usee;
        auto type = base->ty;
        type = to_pointed_type(type);
        auto rs = load_address(base);
        if (!instr->in_this_dim) {
            type = to_elem_type(type);
        }
        int step = type->length();
        auto access = instr->operand(1)->usee;
        if (auto con = dynamic_cast<Ir::Const*>(access)) {
            int imm = std::stoi(con->name());
            int forward = imm * step;
            if (check_itype_immediate(forward)) {
                return {rs, forward};
            }
            auto forwarded = allocate_reg();
            add({ RegRegInstr {
                RegRegInstrType::ADD, forwarded, rs, li(forward)
            } });
            return {forwarded, 0};
        }
        auto index = toReg(access);
        auto forward  = allocate_reg();
        auto forwarded  = allocate_reg();
        if ((step & (step - 1)) == 0) {
            add({ RegImmInstr {
                RegImmInstrType::SLLI, forward, index, __builtin_ctz(step)
            } });
        } else {
            add({ RegRegInstr {
                RegRegInstrType::MUL, forward, li(step), index
            } });
        }
        add({ RegRegInstr {
            RegRegInstrType::ADD, forwarded, rs, forward
        } });
        return {forwarded, 0};
    }

    void convert_mini_gep_instr(const Pointer<Ir::MiniGepInstr> &instr) {
        if (instr->users().size() == 1) {
            if (auto user = dynamic_cast<Ir::Instr*>(instr->users()[0]->user)) {
                if (user->instr_type() == Ir::INSTR_LOAD || user->instr_type() == Ir::INSTR_STORE) {
                    return;
                }
            }
        }
        auto rd = toReg(instr.get());
        auto [reg, offset] = parse_mini_gep(instr.get());
        if (offset == 0) {
            // a hint for register allocator
            add({  RegInstr{
                RegInstrType::MV, rd, reg
            } });
        } else {
            add({ RegImmInstr {
                RegImmInstrType::ADDI, rd, reg, offset,
            } });
        }
    }
#endif

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
            auto index = toReg(instr->operand(dim)->usee);
            if (index != Reg::ZERO) {
                auto forward  = allocate_reg();
                auto forwarded  = allocate_reg();
                if ((step & (step - 1)) == 0) {
                    add({ RegImmInstr {
                        RegImmInstrType::SLLI, forward, index, __builtin_ctz(step)
                    } });
                } else {
                    add({ RegRegInstr {
                        RegRegInstrType::MUL, forward, li(step), index
                    } });
                }
                add({ RegRegInstr {
                    RegRegInstrType::ADD, forwarded, rs, forward
                } });
                rs = forwarded;
            }
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
        if (is_array(type)) {
            auto rd = allocate_reg();
            add({  LoadStackAddressInstr{
                rd, index
            } });
            func.localAddress[index] = rd;
        }
    }

    void convert_jump_instr(const Pointer<Ir::BrInstr>& instr) {
        auto label = instr->operand(0)->usee;
        submit_phi(label, instr->block());
        add({ JInstr{ func.name + "_" + label->name() } });
    }

    void submit_phi(Ir::Val* label, Ir::Block* block) {
        for (auto&& head : *static_cast<Ir::LabelInstr*>(label)->block()) {
            if (dynamic_cast<Ir::LabelInstr*>(head.get())) continue;
            if (auto phi = dynamic_cast<Ir::PhiInstr*>(head.get())) {
                auto flt = is_float(phi->ty);
                auto rd = to_generic_reg(phi);
                for (auto [label, val] : *phi) {
                    if (label == block->label().get()) {
                        auto rs = to_generic_reg(val);
                        if (flt) {
                            add({  FRegInstr{
                                FRegInstrType::FMV_S, std::get<FReg>(rd), std::get<FReg>(rs)
                            } });
                        } else {
                            add({  RegInstr{
                                RegInstrType::MV, std::get<Reg>(rd), std::get<Reg>(rs)
                            } });
                        }
                        break;
                    }
                }
            } else {
                break;
            }
        }
    }

};

MachineInstrs Func::translate(const Ir::pInstr &instr, const Ir::pInstr &next, bool& tail)
{
    ConvertBulk bulk(*this);
    auto block = instr->block();
    switch (instr->instr_type()) {
        case Ir::INSTR_RET:
            bulk.convert_return_instr(std::static_pointer_cast<Ir::RetInstr>(instr));
            tail = true;
            break;
        case Ir::INSTR_BR:
            bulk.convert_jump_instr(std::static_pointer_cast<Ir::BrInstr>(instr));
            tail = true;
            break;
        case Ir::INSTR_BR_COND:
            bulk.convert_branch_instr(std::static_pointer_cast<Ir::BrCondInstr>(instr));
            tail = true;
            break;
        case Ir::INSTR_CALL: {
            auto terminator = *block->rbegin();
            tail = terminator->instr_type() == Ir::INSTR_RET && next == terminator
                    && instr->users().size() == 1 && instr->users().front()->user == terminator.get();
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
                    && instr->users().size() == 1 && instr->users().front()->user == terminator.get();
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
#ifdef USING_MINI_GEP
        case Ir::INSTR_MINI_GEP:
            bulk.convert_mini_gep_instr(std::static_pointer_cast<Ir::MiniGepInstr>(instr));
            break;
#endif
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
    size_t next_arg = 0;
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
                add({ LoadStackInstr {
                    LSType::FLOAT, rd,  next_arg++, StackObjectType::PARENT_ARG,
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
                add({ LoadStackInstr {
                    LSType::DWORD, rd,  next_arg++, StackObjectType::PARENT_ARG,
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

int Func::translate(StackObjectType type, size_t index) const {
    int frame_size = frame.size();
    switch (type) {
        case StackObjectType::LOCAL:
            return frame_size - frame.locals[index].offset;
        case StackObjectType::PARENT_ARG:
            return frame_size + (int)index * 8;
        case StackObjectType::CHILD_ARG:
            return (int)index * 8;
    }
    unreachable();
    return -1;
}

void Func::remove_pseudo() {
    for (auto&& block : blocks) {
        for (auto it = block.body.begin(); it != block.body.end(); ) {
            auto instr = *it;
            if (instr.instr_type() == MachineInstr::Type::LOAD_STACK_ADDRESS) {
                auto LSA = instr.as<LoadStackAddressInstr>();
                int offset = translate(LSA.type, LSA.index);
                auto replacement = spaddi(LSA.rd, offset);
                it = block.body.erase(it);
                it = block.body.insert(it, replacement.begin(), replacement.end());
            } else if (instr.instr_type() == MachineInstr::Type::LOAD_STACK) {
                auto LSA = instr.as<LoadStackInstr>();
                int offset = translate(LSA.type1, LSA.index);
                std::vector<MachineInstr> replacement;
                if (check_itype_immediate(offset)) {
                    replacement.push_back({ LoadInstr {
                        LSA.type, LSA.rd, offset, Reg::SP,
                    } });
                } else {
                    replacement.push_back({ ImmInstr {
                        ImmInstrType::LI, Reg::T0, offset
                    } });
                    replacement.push_back({ RegRegInstr {
                        RegRegInstrType::ADD, Reg::T0, Reg::T0, Reg::SP,
                    } });
                    replacement.push_back({ LoadInstr {
                        LSA.type, LSA.rd, 0, Reg::T0,
                    } });
                }
                it = block.body.erase(it);
                it = block.body.insert(it, replacement.begin(), replacement.end());
            } else if (instr.instr_type() == MachineInstr::Type::STORE_STACK) {
                auto LSA = instr.as<StoreStackInstr>();
                int offset = translate(LSA.type1, LSA.index);
                std::vector<MachineInstr> replacement;
                if (check_itype_immediate(offset)) {
                    replacement.push_back({ StoreInstr {
                        LSA.type, LSA.rs, offset, Reg::SP,
                    } });
                } else {
                    replacement.push_back({ ImmInstr {
                        ImmInstrType::LI, Reg::T0, offset
                    } });
                    replacement.push_back({ RegRegInstr {
                        RegRegInstrType::ADD, Reg::T0, Reg::T0, Reg::SP,
                    } });
                    replacement.push_back({ StoreInstr {
                        LSA.type, LSA.rs, 0, Reg::T0,
                    } });
                }
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

