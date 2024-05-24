#pragma once

#include <utility>


#include "ir_constant.h"
#include "ir_val.h"
#include "type.h"

namespace Ir {

struct Global : public Val {
    Global(const TypedSym& val, Const con, bool is_const = false)
        : Val(make_pointer_type(val.ty)), con(std::move(std::move(con))), is_const(is_const) {
        set_name("@" + val.sym);
    }

    String print_global() const;

    ValType type() const override { return VAL_GLOBAL; }

    Const con;
    bool is_const;
};

using pGlobal = Pointer<Global>;

pGlobal make_global(const TypedSym& val, Const con, bool is_const = false);

} // namespace Ir
