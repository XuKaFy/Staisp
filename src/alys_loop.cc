
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
namespace Alys {

[[nodiscard]] pNaturalLoop make_natural_loop(Ir::Block *header,
                                             Ir::Block *latch) {
    auto ret_loop = std::make_shared<NaturalLoop>(header, latch);
    return std::move(ret_loop);
}

void NaturalLoop::complete_loop() {
    auto &loop_nodes = loop_blocks;
    Vector<Ir::Block *> blk_stk;
    blk_stk.push_back(latch);
    while (!blk_stk.empty()) {
        auto blk = blk_stk.back();
        blk_stk.pop_back();
        if (loop_nodes.insert(blk).second) {
            for (auto pred : blk->in_blocks()) {
                if (pred != header) {
                    blk_stk.push_back(pred);
                }
            }
        }
    }
    loop_nodes.insert(header);
}

LoopInfo::LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx) {
    my_assert(dom_ctx.unreachable_blocks.empty(), "no unreachable blocks");
    Map<Ir::Block *, Set<Ir::Block *>> dom_set;
    auto build_dom_set = [&dom_set,
                          &dom_ctx](const auto &build_dom_set, Ir::Block *bb,
                                    Set<Ir::Block *> idom_dom_set) -> void {
        idom_dom_set.insert(bb);
        dom_set[bb] = idom_dom_set;
        for (auto succ : dom_ctx.dom_map.at(bb)->out_block)
            build_dom_set(build_dom_set, succ->basic_block, idom_dom_set);
    };

    build_dom_set(build_dom_set, dom_ctx.order().front(), Set<Ir::Block *>{});

    for (auto const &bb : dom_ctx.order()) {
        Ir::Block *find = nullptr;
        auto succ_bbs = bb->out_blocks();
        auto &dom_bbs = dom_set.at(bb);
        for (auto succ : succ_bbs) {
            if (dom_bbs.count(succ) > 0) {
                find = succ;
                loops.push_back(make_natural_loop(succ, bb));
            }
        }
    }

    print_loop();
}

inline bool
LoopInfo::is_dom(Ir::Block *a, Ir::Block *b,
                 const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    return dom_set.at(a).count(b) > 0;
}

void LoopInfo::print_loop() const {
    printf("%d: Total NaturalLoop cnts\n", loops.size());
    std::string ret;
    for (auto loop : loops) {
        ret += "loop header: ";
        ret += loop->header->label()->name();
        ret += "loop latch: ";
        ret += loop->latch->label()->name();
        ret += "\n\tloop blks: ";
        for (auto blk : loop->loop_blocks) {
            ret += blk->label()->name() += " ";
        }
        ret += "\n";
    }

    std::cerr << ret;
}
} // namespace Alys