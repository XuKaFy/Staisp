#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

struct Const : public Val {
    Const(Immediate var)
        : Val(), var(var) {}

    virtual Symbol print_impl() const override;
    Immediate var;
};

pVal make_constant(Immediate var);

} // namespace Ir
