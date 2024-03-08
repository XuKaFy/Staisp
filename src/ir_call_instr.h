#pragma once

#include "ir_instr.h"
#include "ir_func.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(pFunc func, Vector<pInstr> args)
        : Instr(INSTR_TYPE_NEED_REG, func->var.tr), func(func), args(args) { }
    
    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }

    pFunc func;
    Vector<pInstr> args;
};

struct RetInstr : public Instr {
    RetInstr(pType tr, pInstr oprd)
        : Instr(INSTR_TYPE_NO_REG, tr), ret(oprd) { }
    
    virtual bool is_end_of_block() const { return true; }

    virtual Symbol instr_print_impl() const override;
    pInstr ret;
};

pInstr make_call_instr(pFunc func, Vector<pInstr> args);
pInstr make_ret_instr(pType tr, pInstr oprd);

} // namespace Ir
