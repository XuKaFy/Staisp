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

    std::vector<GReg> def() const {
        std::vector<GReg> ans;
        for (auto&& instr : body) {
            auto r = instr.def();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }
    std::vector<GReg> use() const {
        std::vector<GReg> ans;
        for (auto&& instr : body) {
            auto r = instr.use();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }
};

} // namespace Backend
