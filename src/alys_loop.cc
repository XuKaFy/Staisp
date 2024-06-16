
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cstdint>
#include <memory>
namespace Alys {

struct LoopHdr {
    Ir::Block *header;
    Ir::Block *latch;

    Ir::Val *next;
    LoopHdr(Ir::Block *h, Ir::Block *l, Ir::Val *n)
        : header(h), latch(l), next(n) {}
};

using pLoopHdr = Pointer<LoopHdr>;

struct loop_list final {
    Vector<pLoopHdr> loops;
    void build_loop(Ir::BlockedProgram &arg_func);
};

auto dom(Ir::Block *a, Ir::Block *b, const DomTree &dom_ctx) -> bool {

    auto dom_b = dom_ctx.dom_map.at(b).get();
    auto dom_a = dom_ctx.dom_map.at(a).get();
    while (dom_b) {
        if (dom_a == dom_b) {
            return true;
        }
        dom_b = dom_b->idom;
    }
    my_assert(dom_b->basic_block == dom_ctx.order().front(), "entry blk");
    return false;
}

void loop_list::build_loop(Ir::BlockedProgram &arg_func) {

    DomTree dom_tree;
    dom_tree.build_dom(arg_func);

    for (auto *cur_blk : dom_tree.order()) {
        auto back_instr = cur_blk->back();
        if (back_instr->instr_type() != Ir::INSTR_BR_COND) {
            continue;
        }

        auto br_instr = std::dynamic_pointer_cast<Ir::BrCondInstr>(back_instr);
        auto cond = br_instr->operands.at(0)->usee;
        auto cmp_cnd = dynamic_cast<Ir::CmpInstr *>(cond);
        if (!cmp_cnd) {
            continue;
        }

        if (!is_integer(cmp_cnd->ty)) {
            continue;
        }

        const auto next = cmp_cnd->operands.at(0)->usee;
        auto bound = cmp_cnd->operands.at(1)->usee;

        Ir::Val *indvar;
        intmax_t step;

        auto true_target =
            dynamic_cast<Ir::LabelInstr *>(br_instr->operand(1)->usee);
        auto head = true_target;

        if (!dom(head->block(), cur_blk, dom_tree)) {
            continue;
        }

        auto cur_loop = std::make_shared<LoopHdr>(head->block(), cur_blk, next);

        loops.push_back(cur_loop);
    }
}

} // namespace Alys