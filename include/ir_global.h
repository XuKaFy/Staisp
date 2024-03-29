#pragma once

#include "type.h"
#include "ir_val.h"
#include "ir_sym.h"
#include "ir_constant.h"

namespace Ir
{

struct Global : public Val {
    Global(TypedSym val, Const con)
        : val(val), con(con) {}
    
    virtual Symbol print_impl() const override;
    Symbol print_global() const;

    TypedSym val;
    Const con;
};

typedef Pointer<Global> pGlobal;

pGlobal make_global(TypedSym val, Const con);

} // namespace ir
