#include "convert_ir_to_asm.h"

#include <ir_call_instr.h>

#include "bkd_global.h"
#include "bkd_ir_instr.h"
#include "bkd_module.h"
#include "ir_instr.h"
#include "value.h"
#include <iterator>
#include <memory>

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
    case Ir::INSTR_BINARY:
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

