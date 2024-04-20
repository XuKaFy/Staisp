#pragma once

#include "ir_instr.h"

namespace Ir {

struct Func : public User {
    Func(TypedSym var, Vector<pType> arg_types)
        : User(make_function_type(var.ty, arg_types)) {
        set_name(var.sym);
    }

    virtual ValType type() const {
        return VAL_FUNC;
    }

    pFunctionType functon_type() const;
};

typedef Pointer<Func> pFunc;

pFunc make_func(TypedSym var, Vector<pType> arg_types);

} // namespace ir
