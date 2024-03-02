#pragma once

#include "ir_func.h"
#include "ir_block.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(ImmType tr, Symbol func_name, Vector<ImmType> arg_types)
        : Func(tr, func_name, arg_types) { }

    virtual Symbol print_impl() const;

    Vector<pBlock> body;
};

typedef Pointer<Func> pFuncDefined;

pFunc make_func_defined(ImmType tr, Symbol func_name, Vector<ImmType> arg_types);

} // namespace ir
