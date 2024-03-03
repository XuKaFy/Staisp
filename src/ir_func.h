#pragma once

#include "ir_instr.h"
#include "ir_sym.h"

namespace Ir {

struct Func : public Val {
    Func(TypedSym var, Vector<TypedSym> arg_types)
        : var(var), args(arg_types) { }

    virtual Symbol print_impl() const override;

    TypedSym var;
    Vector<TypedSym> args;
};

typedef Pointer<Func> pFunc;

pFunc make_func(TypedSym var, Vector<ImmType> arg_types);

} // namespace ir
