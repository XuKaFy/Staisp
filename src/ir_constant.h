#pragma once

#include "def.h"
#include "imm.h"

#include "ir_val.h"

namespace Ir {

struct Const : public Val {
    Const(ImmValue var)
        : Val(), var(var) {}

    virtual Symbol print_impl() const override;
    ImmValue var;
};

pVal make_constant(ImmValue var);

} // namespace Ir
