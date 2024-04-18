#pragma once

#include "ir_instr.h"
#include "ir_func.h"

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(pFunc func, Vector<pVal> args)
        : Instr(to_function_type(func->ty)->ret_type), func_ty(to_function_type(func->ty)) {
        add_operand(func);
        for(auto i : args)
            add_operand(i);
    }
    
    virtual Symbol instr_print_impl() const override;
    pFunctionType func_ty;
};

struct RetInstr : public Instr {
    RetInstr(pVal oprd = {})
        : Instr(make_return_type()) {
        if(oprd)
            add_operand(oprd);
    }

    virtual Symbol instr_print_impl() const override;
};

pInstr make_call_instr(pFunc func, Vector<pVal> args);
pInstr make_ret_instr(pVal oprd = {});

} // namespace Ir
