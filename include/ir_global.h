#pragma once

#include "type.h"
#include "ir_val.h"
#include "ir_constant.h"

namespace Ir
{

struct Global : public Val {
    Global(TypedSym val, Const con)
        : Val(make_pointer_type(val.ty)), con(con) {
        set_name(String("@") + val.sym);
    }
    
    Symbol print_global() const;

    virtual ValType type() const {
        return VAL_GLOBAL;
    }

    Const con;
};

typedef Pointer<Global> pGlobal;

pGlobal make_global(TypedSym val, Const con);

} // namespace ir
