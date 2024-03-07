#pragma once

#include "ir_instr.h"
#include "ir_sym.h"

namespace Ir {

struct Func : public Val {
    Func(ImmTypedSym var, Vector<ImmTypedSym> arg_types)
        : var(var), args(arg_types) { }

    virtual Symbol print_impl() const override;

    ImmTypedSym var;
    Vector<ImmTypedSym> args;
};

typedef Pointer<Func> pFunc;

pFunc make_func(ImmTypedSym var, Vector<ImmType> arg_types);

} // namespace ir
