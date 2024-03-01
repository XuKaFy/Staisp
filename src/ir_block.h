#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

struct Block {
    Vector<Instr> instrs;
};

} // namespace Ir
