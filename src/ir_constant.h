#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

struct Const : public Val {
    Immediate var;
};

} // namespace Ir
