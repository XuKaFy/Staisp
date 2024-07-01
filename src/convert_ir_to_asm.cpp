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

struct ConvertBulk {
    Backend::MachineInstrs bulk;
    int allocate_register = 0;

    void add(Backend::MachineInstr const& value) {
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
                break;
        }
        my_assert(false, "?");
    }

    Backend::Reg toReg(Ir::Val* val, Backend::Reg tmp) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            try {
                return (Backend::Reg) std::stoi(name.substr(1));
            } catch (std::invalid_argument&) {
                return (Backend::Reg) --allocate_register;
            }
        }
        // if it is a immediate, load it
        add({Backend::ImmInstr {
            Backend::ImmInstrType::LI, tmp, std::stoi(name)}
        });
        return tmp;
    }

    Backend::RegRegInstrType selectRRType(ImmType ty, Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return selector(ty,
                    Backend::RegRegInstrType::ADD,
                    Backend::RegRegInstrType::ADDW);
            case Ir::INSTR_SUB:
                return selector(ty,
                    Backend::RegRegInstrType::SUB,
                    Backend::RegRegInstrType::SUBW);
            case Ir::INSTR_MUL:
                return selector(ty,
                    Backend::RegRegInstrType::MUL,
                    Backend::RegRegInstrType::MULW);
            case Ir::INSTR_SDIV:
            case Ir::INSTR_UDIV:
                return selector(ty,
                    Backend::RegRegInstrType::DIV,
                    Backend::RegRegInstrType::DIVW);
            case Ir::INSTR_SREM:
            case Ir::INSTR_UREM:
                return selector(ty,
                    Backend::RegRegInstrType::REM,
                    Backend::RegRegInstrType::REMW);
                break;
            case Ir::INSTR_XOR:
                std::abort();
            case Ir::INSTR_AND:
                return Backend::RegRegInstrType::AND;
            case Ir::INSTR_OR:
                return Backend::RegRegInstrType::OR;
            case Ir::INSTR_ASHR:
                return selector(ty,
                    Backend::RegRegInstrType::SRA,
                    Backend::RegRegInstrType::SRAW);
                case Ir::INSTR_LSHR:
                return selector(ty,
                    Backend::RegRegInstrType::SRL,
                    Backend::RegRegInstrType::SRLW);
            case Ir::INSTR_SHL:
                return selector(ty,
                    Backend::RegRegInstrType::SLL,
                    Backend::RegRegInstrType::SLLW);
        }
        std::abort();
    }


    // note: float is not considered yet



    void convert_binary_instr(const Pointer<Ir::BinInstr> &instr) {
        auto rd = toReg(instr.get(), {});
        auto rs1 = toReg(instr->operand(0)->usee, rd);
        auto rs2 = toReg(instr->operand(1)->usee, rd);
        add({ Backend::RegRegInstr {
            selectRRType(to_basic_type(instr->ty)->ty, instr->binType), rd, rs1, rs2,
        } });
    }

    void convert_return_instr(const Pointer<Ir::RetInstr> &instr) {
        auto ret = std::static_pointer_cast<Ir::RetInstr>(instr);
        if (ret->operand_size() > 0) {
            auto a0 = Backend::Reg::A0;  // a0 is for return value
            add({ Backend::RegInstr{
                Backend::RegInstrType::MV,
                a0,
                toReg(ret->operand(0)->usee, a0)
            } });
        }
        add({ Backend::ReturnInstr{} });
    }

};

Backend::MachineInstrs Convertor::convert(const Ir::pFuncDefined &func, const Ir::pInstr &instr)
{
    ConvertBulk bulk;
    switch (instr->instr_type()) {
    case Ir::INSTR_RET:
        bulk.convert_return_instr(std::dynamic_pointer_cast<Ir::RetInstr>(instr));
        break;
    case Ir::INSTR_BR:
        bulk.add({ Backend::JInstr{ func->name() + "_" + instr->operand(0)->usee->name() } });
        break;
    case Ir::INSTR_BR_COND:
        // condition is not yet proceeded
        bulk.add({ Backend::BranchInstr{Backend::BranchInstrType::BEQ, {}, {}, "TODO" } });
        break;
    case Ir::INSTR_CALL:
        // arguments are not yet proceeded
        bulk.add({ Backend::CallInstr{instr->operand(0)->usee->name()} });
        break;
    case Ir::INSTR_CAST:
    case Ir::INSTR_CMP:
    case Ir::INSTR_STORE:
    case Ir::INSTR_LOAD:
    case Ir::INSTR_UNARY:
        break;
    case Ir::INSTR_BINARY:
        bulk.convert_binary_instr(std::static_pointer_cast<Ir::BinInstr>(instr));
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
    return bulk.bulk;
}

} // namespace BackendConvertor

