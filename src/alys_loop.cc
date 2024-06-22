
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include <functional>
#include <memory>
namespace Alys {

struct NaturalLoop {
    Ir::Block *header;
    Ir::Block *latch;
    Set<Ir::Block *> loop_blocks;
};

using pNaturalLoop = std::shared_ptr<NaturalLoop>;
[[nodiscard]] pNaturalLoop make_natural_loop() {
    return std::make_shared<NaturalLoop>();
}

struct LoopInfo {
    Vector<pNaturalLoop> loops;

    LoopInfo() = delete;
    LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx) {
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

        auto build_loop_from_back_edges = [&dom_set, this](Ir::Block *latch,
                                                           Ir::Block *head) {
            auto loop = make_natural_loop();
            loop->header = head;
            loop->latch = latch;
            loop->loop_blocks.insert(head);
            loop->loop_blocks.insert(latch);
            // to do
            std::function<void(Ir::Block *)> build_loop_blocks =
                [&dom_set, &build_loop_blocks, &loop](Ir::Block *bb) -> void {};
            build_loop_blocks(latch);
            loops.push_back(std::move(loop));
        };
        build_dom_set(p.front().get());

        Vector<Pair<Ir::Block *, Ir::Block *>> back_edges;
        for (auto const &bb : dom_ctx.order()) {
            auto br_instr = bb->back();
            if (br_instr->instr_type() != Ir::INSTR_BR_COND)
                continue;
            auto br = dynamic_cast<Ir::BrCondInstr *>(br_instr.get());
            auto true_label =
                dynamic_cast<Ir::LabelInstr *>(br->operand(1)->usee);
            my_assert(true_label, "label");
            if (!is_dom(bb, true_label->block(), dom_set))
                continue;
            back_edges.push_back({bb, true_label->block()});
            build_loop_from_back_edges(bb, true_label->block());
        }
    }

    // a is dominated by b
    inline bool is_dom(Ir::Block *a, Ir::Block *b,
                       const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
        return dom_set.at(a).count(b);
    }
};
} // namespace Alys