#pragma once

#include "ir_constant.h"
#include "ir_val.h"
#include "type.h"

namespace Ir {

struct Global : public Val {
    Global(TypedSym val, Const con, bool is_const = false)
        : Val(make_pointer_type(val.ty)), con(con), is_const(is_const) {
        set_name(String("@") + val.sym);
    }

    Symbol print_global() const;

    virtual ValType type() const { return VAL_GLOBAL; }

    Const con;
    bool is_const;
};

typedef Pointer<Global> pGlobal;

pGlobal make_global(TypedSym val, Const con, bool is_const = false);

} // namespace Ir
