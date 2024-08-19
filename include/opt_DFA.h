#pragma once

#include "ir_block.h"
#include "opt.h"

namespace Optimize {

template<typename BlockValue>
struct DFAAnalysisData {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    int cnt;
};

template <typename BlockValue, typename TransferFunction>
DFAAnalysisData<BlockValue> from_top_analysis(Ir::BlockedProgram &p, bool do_transfer = true) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    TransferFunction transfer;
    int ans = 0;
    for (const auto &i : p) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = pending_blocks.front();
        pending_blocks.pop_front();

        BlockValue old_OUT = OUTs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        auto in_block = b->in_blocks();
        IN.clear();
        if (!in_block.empty()) {
            IN = OUTs[*in_block.begin()];
            for (auto block : in_block) {
                IN.cup(OUTs[block]);
            }
        }

        OUT = IN;
        transfer(b, OUT); // transfer function

        if (old_OUT != OUT) {
            // printf("    BLOCK CHANGED\n");
            auto out_block = b->out_blocks();
            /*for (auto i : out_block) {
                printf("        NEXT BLOCK: %s\n", i->label()->name().c_str());
            }*/
            pending_blocks.insert(pending_blocks.end(), out_block.begin(), out_block.end());
        } else {
            // printf("    BLOCK UNCHANGED\n");
        }
    }
    if (do_transfer) {
        for (const auto &i : p) {
            ans += transfer(i.get(), INs[i.get()], OUTs[i.get()]);
        }
    }
    return { INs, OUTs, ans };
}

template <typename BlockValue, typename TransferFunction>
DFAAnalysisData<BlockValue> from_bottom_analysis(Ir::BlockedProgram &p, bool do_transfer = true) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    TransferFunction transfer;
    int ans = 0;
    for (const auto &i : p) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = *pending_blocks.begin();
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
    if (do_transfer) {
        for (const auto &i : p) {
            ans += transfer(i.get(), INs[i.get()], OUTs[i.get()]);
        }
    }
    return { INs, OUTs, ans };
}

}