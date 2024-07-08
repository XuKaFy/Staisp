
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
    Map<Ir::Block *, Set<Ir::Block *>> dom_set = build_dom_set(dom_ctx);

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

auto build_dom_set(const DomTree &dom_ctx)
    -> Map<Ir::Block *, Set<Ir::Block *>> {
    Map<Ir::Block *, Set<Ir::Block *>> dom_set;

    auto cur_bb = dom_ctx.order().front();

    std::function<void(Ir::Block *, Set<Ir::Block *>)> dfs_builder =
        [&dom_set, &dfs_builder, &dom_ctx](Ir::Block *cur_bb,
                                           Set<Ir::Block *> idom_doms) -> void {
        idom_doms.insert(cur_bb);
        dom_set[cur_bb] = idom_doms;
        for (auto dom_succ : dom_ctx.dom_map.at(cur_bb)->out_block)
            dfs_builder(dom_succ->basic_block, idom_doms);
    };
    dfs_builder(cur_bb, Set<Ir::Block *>{});
    return dom_set;
}
} // namespace Alys