#pragma once

#include "ir_val.h"
#include "ir_sym.h"
#include "ir_constant.h"

namespace Ir
{

struct Global : public Val {
    Global(ImmType tr, Const con, pSym name)
        : tr(tr), con(con), name(name) {}
    
    virtual Symbol print_impl() const override;
    Symbol print_global() const;

    ImmType tr;
    Const con;
    pSym name;
};

typedef Pointer<Global> pGlobal;

pGlobal make_global(ImmType tr, Const con, pSym name);

} // namespace ir
