#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "ir_val.h"
#include "value.h"

namespace Ir {

struct Const : public Val {
    Const(Value var)
        : Val(var.ty), v(var) {
        set_name(var.to_string());
    }
    
    Value v;
};

pVal make_constant(Value var);

} // namespace Ir
