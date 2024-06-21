#pragma once

#include "def.h"
#include "bkd_ir_instr.h"
#include <memory>

namespace Backend {

struct Block {
    explicit Block(std::string name)
        : label(std::make_shared<LabelInstr>(std::move(name))) {}

    String print(std::string const& function_name) const;

    pLabelInstr label;
    MachineInstrs body;
};

} // namespace Backend
