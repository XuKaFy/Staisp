#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "ir_val.h"

namespace Ir {

struct Const : public Val {
    Const(ImmValue var);
    Const(pType ty);

    virtual Symbol print_impl() const override;
    pType ty;
    Memory var_mem;
};

pVal make_constant(ImmValue var);
pVal make_constant(pType ty);

} // namespace Ir
