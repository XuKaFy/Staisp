#pragma once

#include "ir_instr.h"
#include "ir_sym.h"

namespace Ir {

struct Func : public Val {
    Func(ImmType tr, pSym func_name, Vector<TypedSym> arg_types)
        : tr(tr), func_name(func_name), args(arg_types) { }

    virtual Symbol print_impl() const override;

    ImmType tr;
    pSym func_name;
    Vector<TypedSym> args;
};

typedef Pointer<Func> pFunc;

pFunc make_func(ImmType tr, pSym func_name, Vector<ImmType> arg_types);

} // namespace ir
