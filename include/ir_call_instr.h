#pragma once

#include "ir_func.h"
#include "ir_instr.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(const pFunc &func, const Vector<pVal> &args)
        : Instr(to_function_type(func->ty)->ret_type),
          func_ty(to_function_type(func->ty)) {
        add_operand(func);
        for (const auto &i : args) {
            add_operand(i);
        }
    }

    InstrType instr_type() const override { return INSTR_CALL; }

    String instr_print() const override;

    pFunctionType func_ty;
};

struct RetInstr : public Instr {
    RetInstr(const pVal &oprd = {}) : Instr(make_ir_type(IR_RET)) {
        if (oprd) {
            add_operand(oprd);
        }
    }

    InstrType instr_type() const override { return INSTR_RET; }

    String instr_print() const override;
};

struct UnreachableInstr : Instr {
    UnreachableInstr() : Instr(make_ir_type(IR_UNREACHABLE)) {}

    InstrType instr_type() const override { return INSTR_UNREACHABLE; }

    String instr_print() const override { return "unreachable"; }
};

pInstr make_call_instr(const pFunc &func, const Vector<pVal> &args);
pInstr make_ret_instr(const pVal &oprd = {});
pInstr make_unreachable_instr();

} // namespace Ir
