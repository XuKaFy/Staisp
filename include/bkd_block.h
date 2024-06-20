#pragma once

#include "def.h"
#include "bkd_ir_instr.h"
#include <memory>

namespace Backend {

struct Block {
    Block()
        : label(std::make_shared<LabelInstr>()) {}

    String print() const;

    pLabelInstr label;
    MachineInstrs body;
};

} // namespace Backend
