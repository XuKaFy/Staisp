#pragma once

#include "def.h"
#include "bkd_instr.h"

namespace Backend {

struct Block {
    explicit Block(std::string name = ":placeholder:")
        : name(std::move(name)) {}

    String print() const;

    std::string name;
    MachineInstrs body;

    std::vector<Reg> def() const {
        std::vector<Reg> ans;
        for (auto&& instr : body) {
            auto r = instr.def();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }
    std::vector<Reg> use() const {
        std::vector<Reg> ans;
        for (auto&& instr : body) {
            auto r = instr.use();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }
    std::vector<FReg> fdef() const {
        std::vector<FReg> ans;
        for (auto&& instr : body) {
            auto r = instr.fdef();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }
    std::vector<FReg> fuse() const {
        std::vector<FReg> ans;
        for (auto&& instr : body) {
            auto r = instr.fuse();
            ans.insert(ans.end(), r.begin(), r.end());
        }
        return ans;
    }

    std::vector<Block*> out_blocks() const {
        return {};
    }

    std::vector<Block*> in_blocks() const {
        return {};
    }
};

} // namespace Backend
