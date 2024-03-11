#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "ir_val.h"
#include "value.h"

namespace Ir {

struct Const : public Val {
    Const(Value var)
        : v(var) { }
    Const(ImmValue var)
        : v(var) { }
    Const(PointerValue var)
        : v(var) { }
    Const(ArrayValue var)
        : v(var) { }
    Const(StructValue var)
        : v(var) { }
    
    virtual Symbol print_impl() const override;
    Value v;
};

pVal make_constant(ImmValue var);
pVal make_constant(PointerValue var);
pVal make_constant(ArrayValue var);
pVal make_constant(StructValue var);

} // namespace Ir
