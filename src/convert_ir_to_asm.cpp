#include "convert_ir_to_asm.h"

#include <cstddef>
#include <ir_call_instr.h>

#include "bkd_global.h"
#include "bkd_ir_instr.h"
#include "bkd_module.h"
#include "bkd_reg.h"
#include "bkd_regreginstrtype.h"
#include "def.h"
#include "imm.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "type.h"
#include "value.h"
#include <iterator>
#include <memory>
#include <string>

namespace IrToAsm {

Backend::pModule Convertor::convert(const Ir::pModule &mod)
{
    auto bkd_mod = std::make_shared<Backend::Module>();

    for (auto &&i : mod->globs) {
        bkd_mod->globs.push_back(convert(i));
    }
    
    for (auto &&i : mod->funsDefined) {
        bkd_mod->funcs.push_back(convert(i));
    }
    
    return bkd_mod;
}

Backend::Global Convertor::convert(const Ir::pGlobal &glob)
{
    Value& con = glob->con.v;
    switch (con.type()) {
    case VALUE_IMM:
        // all imm should be regard as int32
        return Backend::Global(glob->name().substr(1), con.imm_value().val.ival % 2147483648);
    case VALUE_ARRAY:
        return Backend::Global(glob->name().substr(1), con.array_value());
    }
    my_assert(false, "?");
    return Backend::Global("", 0);
}

Backend::Func Convertor::convert(const Ir::pFuncDefined &func)
{
    Backend::Func bkd_func(func->name());

    for (auto &&i : func->p) {
        bkd_func.body.push_back(convert(func, i));
    }

    return bkd_func;
}

Backend::Block Convertor::convert(const Ir::pFuncDefined &func, const Ir::pBlock &block)
{
    Backend::Block bkd_block(func->name() + "_" + block->label()->name());

    for (auto &&i : *block) {
        auto res = convert(func, i);
        bkd_block.body.insert(bkd_block.body.end(),
            std::move_iterator(res.begin()), std::move_iterator(res.end()));
        res.clear();
    }

    return bkd_block;
}

template<typename T>
T selector(ImmType ty,
    T i32_use,
    T i64_use,
    T u32_use,
    T u64_use)
{
    switch (ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
        return i32_use;
    case IMM_I64:
        return i64_use;
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
        return u32_use;
    case IMM_U64:
        return u64_use;
    default:
        break;
    }
    my_assert(false, "?");
}

template<typename T>
T selector(ImmType ty,
    T f32_use,
    T f64_use)
{
    switch (ty) {
    case IMM_F32:
        return f32_use;
    case IMM_F64:
        return f64_use;
    default: break;
    }
    my_assert(false, "?");
}

Backend::MachineInstr selector(
    ImmType ty, int rd, int rs, int rt,
    Backend::FRegFRegInstrType r_f32_use,
    Backend::FRegFRegInstrType r_f64_use) {
    auto i = Backend::FRegFRegInstr {
        selector(ty, r_f32_use, r_f64_use),
        (Backend::FReg) rd, (Backend::FReg) rs, (Backend::FReg) rt };
    return Backend::MachineInstr {i};
}

Backend::MachineInstr selector(
    ImmType ty, int rd, int rs, int rt,
    Backend::RegRegInstrType r_i32_use,
    Backend::RegRegInstrType r_i64_use,
    Backend::RegRegInstrType r_u32_use,
    Backend::RegRegInstrType r_u64_use) {
    auto i = Backend::RegRegInstr {
        selector(ty, r_i32_use, r_i64_use, r_u32_use, r_u64_use),
        (Backend::Reg) rd, (Backend::Reg) rs, (Backend::Reg) rt };
    return Backend::MachineInstr {i};
}

Backend::MachineInstr selector(
    ImmType ty, int rd, int rs, int rt, int imm,
    Backend::RegRegInstrType r_i32_use,
    Backend::RegRegInstrType r_i64_use,
    Backend::RegRegInstrType r_u32_use,
    Backend::RegRegInstrType r_u64_use,
    Backend::RegImmInstrType i_i32_use,
    Backend::RegImmInstrType i_i64_use,
    Backend::RegImmInstrType i_u32_use,
    Backend::RegImmInstrType i_u64_use) {
    if (rt != -1) {
        auto i = Backend::RegRegInstr {
            selector(ty, r_i32_use, r_i64_use, r_u32_use, r_u64_use),
            (Backend::Reg) rd, (Backend::Reg) rs, (Backend::Reg) rt };
        return Backend::MachineInstr {i};
    }
    auto i = Backend::RegImmInstr {
            selector(ty, i_i32_use, i_i64_use, i_u32_use, i_u64_use),
            (Backend::Reg) rd, (Backend::Reg) rs, imm };
    return Backend::MachineInstr {i};
}

int to_int(std::string name)
{
    int a = 0;
    for (auto i = name.rbegin(); i != std::prev(name.rend()); ++i) {
        a = a * 10 + *i;
    };
    return a;
}

int string_to_reg(std::string name)
{
    if (name[0] != '%') {
        return -1; // imm
    }
    return to_int(name.substr(1));
}

Backend::MachineInstr convert_binary_instr(const Pointer<Ir::BinInstr> &instr)
{
    my_assert(instr, "?");
    switch (instr->binType) {
    case Ir::INSTR_ADD:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            to_int(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::ADD,
            Backend::RegRegInstrType::ADDW,
            Backend::RegRegInstrType::ADD,
            Backend::RegRegInstrType::ADDW,
            Backend::RegImmInstrType::ADDI,
            Backend::RegImmInstrType::ADDIW,
            Backend::RegImmInstrType::ADDI,
            Backend::RegImmInstrType::ADDIW);
    case Ir::INSTR_SUB:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            -to_int(instr->operand(1)->usee->name()), // imm is negative
            Backend::RegRegInstrType::SUB,
            Backend::RegRegInstrType::SUBW,
            Backend::RegRegInstrType::SUB,
            Backend::RegRegInstrType::SUBW,
            Backend::RegImmInstrType::ADDI,
            Backend::RegImmInstrType::ADDIW,
            Backend::RegImmInstrType::ADDI,
            Backend::RegImmInstrType::ADDIW);
    case Ir::INSTR_MUL:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::MUL,
            Backend::RegRegInstrType::MULW,
            Backend::RegRegInstrType::MUL,
            Backend::RegRegInstrType::MULW);
    case Ir::INSTR_FADD:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::FRegFRegInstrType::FADD_S,
            Backend::FRegFRegInstrType::FADD_S);
    case Ir::INSTR_FSUB:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::FRegFRegInstrType::FSUB_S,
            Backend::FRegFRegInstrType::FSUB_S);
    case Ir::INSTR_FMUL:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::FRegFRegInstrType::FMUL_S,
            Backend::FRegFRegInstrType::FMUL_S);
    case Ir::INSTR_SDIV:
    case Ir::INSTR_UDIV:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::DIV,
            Backend::RegRegInstrType::DIVW,
            Backend::RegRegInstrType::DIV,
            Backend::RegRegInstrType::DIVW);
    case Ir::INSTR_SREM:
    case Ir::INSTR_UREM:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::REM,
            Backend::RegRegInstrType::REMW,
            Backend::RegRegInstrType::REM,
            Backend::RegRegInstrType::REMW);
        break;
    case Ir::INSTR_FDIV:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::FRegFRegInstrType::FDIV_S,
            Backend::FRegFRegInstrType::FDIV_S);
    case Ir::INSTR_FREM:
        break;
    case Ir::INSTR_XOR:
    case Ir::INSTR_AND:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::AND,
            Backend::RegRegInstrType::AND,
            Backend::RegRegInstrType::AND,
            Backend::RegRegInstrType::AND);
    case Ir::INSTR_OR:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::OR,
            Backend::RegRegInstrType::OR,
            Backend::RegRegInstrType::OR,
            Backend::RegRegInstrType::OR);
    case Ir::INSTR_ASHR:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::SRA,
            Backend::RegRegInstrType::SRAW,
            Backend::RegRegInstrType::SRA,
            Backend::RegRegInstrType::SRAW);
        case Ir::INSTR_LSHR:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::SRL,
            Backend::RegRegInstrType::SRLW,
            Backend::RegRegInstrType::SRL,
            Backend::RegRegInstrType::SRLW);
    case Ir::INSTR_SHL:
        return selector(to_basic_type(instr->ty)->ty,
            string_to_reg(instr->name()),
            string_to_reg(instr->operand(0)->usee->name()),
            string_to_reg(instr->operand(1)->usee->name()),
            Backend::RegRegInstrType::SLL,
            Backend::RegRegInstrType::SLLW,
            Backend::RegRegInstrType::SLL,
            Backend::RegRegInstrType::SLLW);
        break;
    }
    std::abort(); // no test case should reach here
}

Backend::MachineInstrs Convertor::convert(const Ir::pFuncDefined &func, const Ir::pInstr &instr)
{
    Backend::MachineInstrs res;
    switch (instr->instr_type()) {
    case Ir::INSTR_RET:
        // return value is not yet proceeded
        res.push_back({ Backend::ReturnInstr{} });
        break;
    case Ir::INSTR_BR:
        // label is not yet proceeded
        res.push_back({ Backend::JInstr{ func->name() + "_" + instr->operand(0)->usee->name() } });
        break;
    case Ir::INSTR_BR_COND:
        // condition is not yet proceeded
        res.push_back({ Backend::BranchInstr{Backend::BranchInstrType::BEQ, {}, {}, "TODO" } });
        break;
    case Ir::INSTR_CALL:
        // arguments are not yet proceeded
        res.push_back({ Backend::CallInstr{instr->operand(0)->usee->name()} });
        break;
    case Ir::INSTR_CAST:
    case Ir::INSTR_CMP:
    case Ir::INSTR_STORE:
    case Ir::INSTR_LOAD:
    case Ir::INSTR_UNARY:
        break;
    case Ir::INSTR_BINARY:
        res.push_back(convert_binary_instr(std::static_pointer_cast<Ir::BinInstr>(instr)));
        break;
    case Ir::INSTR_ITEM:
        break;
    case Ir::INSTR_FUNC:
    case Ir::INSTR_ALLOCA:
    case Ir::INSTR_LABEL:
    case Ir::INSTR_PHI:
    case Ir::INSTR_SYM:
    case Ir::INSTR_UNREACHABLE:
        // just ignore all these ir
        break;
    }
    return res;
}

} // namespace BackendConvertor

