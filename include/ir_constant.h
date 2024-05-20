#pragma once

#include "type.h"

#include "ir_val.h"
#include "value.h"

namespace Ir {

struct Const : public Val {
    Const(Value var) : Val(var.ty), v(var) { set_name(var.to_string()); }

    virtual ValType type() const { return VAL_CONST; }

    Value v;
};

typedef Pointer<Const> pConst;

pVal make_constant(Value var);

} // namespace Ir
