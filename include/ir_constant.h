#pragma once

#include "type.h"

#include "ir_val.h"
#include "value.h"

namespace Ir {

struct Const : public Val {
    Const(const Value& var) : Val(var.ty), v(var) { set_name(var.to_string()); }

    ValType type() const override { return VAL_CONST; }

    Value v;
};

using pConst = Pointer<Const>;

pVal make_constant(const Value& var);

} // namespace Ir
