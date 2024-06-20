#pragma once

#include "def.h"
#include "bkd_ir_instr.h"

namespace Backend {

struct Block {
    String print() const;

    pLabelInstr label;
    MachineInstrs body;
};

} // namespace Backend
