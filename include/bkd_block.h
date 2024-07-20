#pragma once

#include "def.h"
#include "bkd_instr.h"

namespace Backend {

struct Block {
    explicit Block(std::string name)
        : name(std::move(name)) {}

    String print() const;

    std::string name;
    MachineInstrs body;
    std::vector<Block*> in_blocks, out_blocks;
    std::unordered_set<GReg> defs, uses;
};

} // namespace Backend
