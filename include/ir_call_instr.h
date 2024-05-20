#pragma once

#include "ir_func.h"
#include "ir_instr.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(pFunc func, Vector<pVal> args)
        : Instr(to_function_type(func->ty)->ret_type),
          func_ty(to_function_type(func->ty)) {
        add_operand(func);
        for (auto i : args)
            add_operand(i);
    }

    virtual InstrType instr_type() const override { return INSTR_CALL; }

    virtual Symbol instr_print_impl() const override;

    pFunctionType func_ty;
};

struct RetInstr : public Instr {
    RetInstr(pVal oprd = {}) : Instr(make_ir_type(IR_RET)) {
        if (oprd)
            add_operand(oprd);
    }

    virtual InstrType instr_type() const override { return INSTR_RET; }

    virtual Symbol instr_print_impl() const override;
};

pInstr make_call_instr(pFunc func, Vector<pVal> args);
pInstr make_ret_instr(pVal oprd = {});

} // namespace Ir
