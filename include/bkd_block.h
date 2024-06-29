#pragma once

#include "def.h"
#include "bkd_ir_instr.h"
#include <memory>

namespace Backend {

struct Block {
    explicit Block(std::string name)
        : name(std::move(name)) {}

    String print() const;

    std::string name;
    MachineInstrs body;
};

} // namespace Backend
