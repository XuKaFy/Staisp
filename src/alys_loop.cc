
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "trans_loop.h"
#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
namespace Alys {

[[nodiscard]] pNaturalLoopBody
make_natural_loop(Ir::Block *header, Ir::Block *latch,
                  const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    return std::make_shared<NaturalLoopBody>(header, latch, dom_set);
}

void NaturalLoopBody::complete_loop(
    Ir::Block *latch, const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    auto &loop_nodes = loop_blocks;
    Vector<Ir::Block *> blk_stk;
    blk_stk.push_back(latch);
    while (!blk_stk.empty()) {
        auto blk = blk_stk.back();
        blk_stk.pop_back();
        if (loop_nodes.insert(blk).second) {
            for (auto pred : blk->in_blocks()) {
                if (pred != header && is_dom(pred, header, dom_set)) {
                    blk_stk.push_back(pred);
                }
            }
        }
    }
    loop_nodes.insert(header);
}

void NaturalLoopBody::handle_indvar(
    const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) & {
    auto back_instr = header->back().get();
    if (back_instr->instr_type() != Ir::INSTR_BR_COND) {
        ind = nullptr;
    } else {

        auto loop_cnd =
            dynamic_cast<Ir::BrCondInstr *>(back_instr)->operand(0)->usee;
        loop_cnd_instr = dynamic_cast<Ir::CmpInstr *>(loop_cnd);

        Ir::Val *lhs;
        Ir::Val *rhs;

        auto is_invariant =
            [&dom_set, this](Ir::Val *cur_val, Ir::Val *&checker_val) -> bool {
            auto cur_val_instr = dynamic_cast<Ir::Instr *>(cur_val);
            checker_val = cur_val;
            if (!cur_val_instr)
                return my_assert(cur_val->type() == Ir::VAL_CONST,
                                 "Const as value"),
                       true;
            if (cur_val_instr->block() != header &&
                is_dom(header, cur_val_instr->block(), dom_set)) {
                return true;
            }
            return false;
        };

        auto is_not_invariant =
            [&is_invariant](Ir::Val *cur_val, Ir::Val *&checker_val) -> bool {
            return !is_invariant(cur_val, checker_val);
        };
        if (auto is_icmp = !is_float(loop_cnd_instr->operand(0)->usee->ty);
            is_icmp) {
            if (Optimize::cmp_extractor<Ir::Val *, Ir::Val *, true>(
                    loop_cnd_instr, cmp_op, lhs, rhs, is_not_invariant,
                    is_invariant)) {
                ind = lhs;
                bound = rhs;
            } else if (Optimize::cmp_extractor<Ir::Val *, Ir::Val *, true>(
                           loop_cnd_instr, cmp_op, lhs, rhs, is_invariant,
                           is_not_invariant)) {
                ind = rhs;
                bound = lhs;
            } else {
                ind = bound = nullptr;
            }
        } else {
            if (Optimize::cmp_extractor<Ir::Val *, Ir::Val *, false>(
                    loop_cnd_instr, cmp_op, lhs, rhs, is_not_invariant,
                    is_invariant)) {
                ind = lhs;
                bound = rhs;
            } else if (Optimize::cmp_extractor<Ir::Val *, Ir::Val *, false>(
                           loop_cnd_instr, cmp_op, lhs, rhs, is_invariant,
                           is_not_invariant)) {
                ind = rhs;
                bound = lhs;
            } else {
                ind = bound = nullptr;
            }
        }
    }
    // if (ind)
    //     std::cerr << "induction var: "
    //               << dynamic_cast<Ir::Instr *>(ind)->instr_print() <<
    //               std::endl;
}

LoopInfo::LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx) {
    my_assert(dom_ctx.unreachable_blocks.empty(), "no unreachable blocks");
    Map<Ir::Block *, Set<Ir::Block *>> dom_set = build_dom_set(dom_ctx);

    for (auto const &bb : dom_ctx.order()) {
        auto succ_bbs = bb->out_blocks();
        auto &dom_bbs = dom_set.at(bb);
        for (auto succ : succ_bbs) {
            if (dom_bbs.count(succ) > 0) {
                if (loops.count(succ) > 0) {
                    loops[succ]->complete_loop(bb, dom_set);
                } else {
                    auto cur_loop = loops[succ] =
                        make_natural_loop(succ, bb, dom_set);
                    cur_loop->handle_indvar(dom_set);
                }
            }
        }
    }

    // print_loop();
}


std::string NaturalLoopBody::print() {
    std::string ret;
    ret += "loop header: ";
    ret += header->label()->name();
    if (ind) {
        ret += "\ninduction var: ";
        ret += dynamic_cast<Ir::Instr *>(ind)->instr_print();
        ret += "\nbounding val: ";
        ret += bound->name();
        ret += "\nrange operator: ";
        ret += Ir::gCmpInstrName[cmp_op];
    }
    ret += "\nloop blks: ";
    for (auto blk : loop_blocks) {
        ret += blk->label()->name() += " ";
    }
    ret += "\n";
    return ret;
}


void LoopInfo::print_loop() const {
    printf("%zu: Total NaturalLoop cnts\n", loops.size());
    std::string ret;
    for (auto&& [loop_hdr, loop_body] : loops) {
        ret += loop_body->print();
    }
    std::cerr << ret << std::flush;
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