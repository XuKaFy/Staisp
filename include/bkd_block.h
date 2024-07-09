#pragma once

#include <bkd_module.h>

#include "def.h"
#include "bkd_instr.h"
#include <memory>

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

// copied from DFA, for liveness analysis
template <typename BlockValue, typename TransferFunction>
int from_bottom_analysis(Func &p) {
    Map<Block *, BlockValue> INs;
    Map<Block *, BlockValue> OUTs;
    std::deque<Block *> pending_blocks;
    TransferFunction transfer;
    int ans = 0;
    for (const auto &i : p.body) {
        INs[&i] = BlockValue();
        OUTs[&i] = BlockValue();
        pending_blocks.push_back(&i);
    }
    while (!pending_blocks.empty()) {
        Block *b = *pending_blocks.begin();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks();
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        transfer(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }
    for (const auto &i : p.body) {
        ans += transfer(i, INs[&i], OUTs[&i]);
    }
    return ans;
}


} // namespace Backend
