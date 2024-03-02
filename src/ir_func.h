#pragma once

#include "ir_instr.h"

namespace Ir {

struct Func : public Val {
    Func(ImmType tr, Symbol func_name, Vector<ImmType> arg_types)
        : tr(tr), func_name(func_name), arg_types(arg_types) { }

    virtual Symbol print_impl() const;

    Symbol func_name;
    Vector<ImmType> arg_types;
    ImmType tr;
};

typedef Pointer<Func> pFunc;

pFunc make_func(ImmType tr, Symbol func_name, Vector<ImmType> arg_types);

} // namespace ir
