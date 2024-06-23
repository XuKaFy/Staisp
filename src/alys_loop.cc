
#include "alys_loop.h"
namespace Alys {

[[nodiscard]] pNaturalLoop make_natural_loop() {
    return std::make_shared<NaturalLoop>();
}

LoopInfo::LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx) {
    my_assert(dom_ctx.unreachable_blocks.empty(), "no unreachable blocks");
    Map<Ir::Block *, Set<Ir::Block *>> dom_set;
    std::function<void(Ir::Block *)> build_dom_set =
        [&dom_ctx, &dom_set, &build_dom_set](Ir::Block *bb) -> void {
        auto dom_bb = dom_ctx.dom_map.at(bb);
        auto idom = dom_bb->idom;
        auto idom_dom_set =
            idom ? dom_set[idom->basic_block] : Set<Ir::Block *>{};
        idom_dom_set.insert(bb);
        dom_set.insert({bb, idom_dom_set});
        for (auto dom_succ : dom_bb->out_block) {
            build_dom_set(dom_succ->basic_block);
        }
    };

    auto build_loop_from_back_edges = [this](Ir::Block *latch,
                                             Ir::Block *head) {
        auto loop = make_natural_loop();
        loop->latch = latch;
        loop->header = head;
        auto &loop_nodes = loop->loop_blocks;
        loop_nodes.insert(latch);
        Vector<Ir::Block *> blk_stk;
        blk_stk.push_back(latch);
        while (!blk_stk.empty()) {
            auto blk = blk_stk.back();
            blk_stk.pop_back();
            loop->loop_blocks.insert(blk);
            if (loop_nodes.insert(blk).second) {
                for (auto pred : blk->in_blocks()) {
                    if (pred != head) {
                        blk_stk.push_back(pred);
                    }
                }
            }
        }
        loop_nodes.insert(head);
        loops.push_back(std::move(loop));
    };
    build_dom_set(p.front().get());

    Vector<Pair<Ir::Block *, Ir::Block *>> back_edges;
    for (auto const &bb : dom_ctx.order()) {
        auto br_instr = bb->back();
        if (br_instr->instr_type() != Ir::INSTR_BR_COND)
            continue;
        auto br = dynamic_cast<Ir::BrCondInstr *>(br_instr.get());
        auto true_label = dynamic_cast<Ir::LabelInstr *>(br->operand(1)->usee);
        my_assert(true_label, "label");
        if (!is_dom(bb, true_label->block(), dom_set))
            continue;
        back_edges.push_back({bb, true_label->block()});
        build_loop_from_back_edges(bb, true_label->block());
    }

    print_loop();
}

inline bool
LoopInfo::is_dom(Ir::Block *a, Ir::Block *b,
                 const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    return dom_set.at(a).count(b);
}

void LoopInfo::print_loop() const {
    for (auto loop : loops) {
        printf("Loop header: %s, latch: %s\n", loop->header->name().c_str(),
               loop->latch->name().c_str());
        puts("\tloop blks: ");
        for (auto blk : loop->loop_blocks) {
            printf("%s ", blk->name().c_str());
        }
        puts("");
    }
}
} // namespace Alys