#pragma once

#include "ir_instr.h"
#include "ir_func.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(ImmType tr, pFunc func, Vector<pVal> args)
        : Instr(INSTR_TYPE_NO_REG, tr), func(func), args(args) { }
    
    virtual Symbol instr_print_impl() const override;

    pFunc func;
    Vector<pVal> args;
};

struct RetInstr : public Instr {
    RetInstr(ImmType tr, pVal oprd)
        : Instr(INSTR_TYPE_NO_REG, tr), ret(oprd) { }
    
    virtual Symbol instr_print_impl() const override;
    pVal ret;
};

pInstr make_call_instr(ImmType tr, pFunc func, Vector<pVal> args);
pInstr make_ret_instr(ImmType tr, pVal oprd);

} // namespace Ir
