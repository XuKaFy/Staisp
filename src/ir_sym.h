#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir
{

struct Sym : public Val {
    Sym(Symbol name)
        : name(name) { }
    virtual Symbol print_impl() const override;

    Symbol name;
};

typedef Pointer<Sym> pSym;

pSym make_sym(Symbol name);

} // namespace ir
