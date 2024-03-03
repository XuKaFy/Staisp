#pragma once

#include "ir_instr.h"
#include "ir_func.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(pFunc func, Vector<pInstr> args)
        : Instr(INSTR_TYPE_NEED_REG, func->var.tr), func(func), args(args) { }
    
    virtual Symbol instr_print_impl() const override;

    pFunc func;
    Vector<pInstr> args;
};

struct RetInstr : public Instr {
    RetInstr(ImmType tr, pInstr oprd)
        : Instr(INSTR_TYPE_NO_REG, tr), ret(oprd) { }
    
    virtual Symbol instr_print_impl() const override;
    pInstr ret;
};

pInstr make_call_instr(pFunc func, Vector<pInstr> args);
pInstr make_ret_instr(ImmType tr, pInstr oprd);

} // namespace Ir
