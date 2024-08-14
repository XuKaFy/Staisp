#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include <algorithm>
#include <cassert>
#include <cstdio>

#include <memory>
#include <string>

namespace Alys {

pDomBlock DomTree::make_domblk() { return std::make_shared<DomBlock>(); }

auto DomTree::build_dfsn(Set<DomBlock *> &v, DomBlock *cur) -> void {
    my_assert(!v.count(cur), "tree");
    v.insert(cur);
    std::sort(cur->out_block.begin(), cur->out_block.end(),
              [](const DomBlock *a, const DomBlock *b) -> bool {
                  return a->basic_block->label()->name() <
                         b->basic_block->label()->name();
              });
    dom_order.push_back(cur->basic_block);
    unreachable_blocks.erase(cur->basic_block);
    for (auto *out : cur->out_block) {
        build_dfsn(v, out);
    }
}

void DomTree::build_dom(Ir::BlockedProgram &p) {
    Map<Ir::Block *, int> order_map;

    for (const auto &bb : p) {
        auto [it, success] = dom_map.insert({bb.get(), make_domblk()});
        auto dom_it = it->second;
        my_assert(success, "insertion success");
        dom_it->basic_block = bb.get();
        order_map.insert({bb.get(), 0});
    }

    int level = 1;
    Vector<Ir::Block *> idfn;
    auto dfsn = [&idfn, &order_map](auto dfsn, Ir::Block *bb,
                                    int &level) -> void {
        if (order_map.at(bb) == level) {
            return;
        }
        if (level == 1) {
            idfn.push_back(bb);
        }
        order_map.at(bb) = level;
        for (Ir::Block *succ : bb->out_blocks()) {
            dfsn(dfsn, succ, level);
        }
    };

    auto entry = p.front().get();
    dfsn(dfsn, entry, level);
    for (auto node : idfn) {
        level++;
        order_map.at(node) = level;
        my_assert(level != 1, "idfn will not changed");
        dfsn(dfsn, entry, level);
        for (auto &[bb, dom_bb] : dom_map) {
            if (auto b_lvl = order_map.at(bb); b_lvl != level && b_lvl >= 1) {
                dom_bb->idom = dom_map.at(node).get();
            }
        }
    }

    for (auto &[bb, dom_bb] : dom_map) {
        if (dom_bb->idom != nullptr) {
            dom_bb->idom->out_block.push_back(dom_bb.get());
        }
        unreachable_blocks.insert(bb);
    }
    Set<DomBlock *> v;
    build_dfsn(v, dom_map[entry].get());
    dom_set = build_dom_set(*this);
    for (auto &[bb, dom_bb] : dom_map) {
        if (v.count(dom_bb.get())) {
            my_assert(!unreachable_blocks.count(bb), y);
        } else {
            my_assert(unreachable_blocks.count(bb), y);
        }
    }
}

void DomTree::print_dom_tree() const {

    auto get_name = [](const DomBlock *dom_bb) -> std::string {
        return std::string{dom_bb->basic_block->label()->name()};
    };
    for (const auto &[dom_bb, dom_node] : dom_map) {
        printf("Dominance Tree: Block %s\n", get_name(dom_node.get()).c_str());
        if (dom_node->idom != nullptr)
            printf("\tIdom: %s\n", get_name(dom_node->idom).c_str());
        printf("\tOut Block: ");
        for (auto *j : dom_node->out_block) {
            printf("\t%s ;", get_name(j).c_str());
            my_assert(dom_map.at(j->basic_block)->idom == dom_node.get(),
                      "dominance tree success");
        }
        puts("\n\n");
    }
};

Map<Ir::Block *, pDomBlock> DomTree::build_dom_frontier() const {
    Map<Ir::Block *, pDomBlock> dom_frontier;
    for (const auto &[bb, dom_node] : dom_map) {
        for (auto in : bb->in_blocks()) {
            Ir::Block *runner = in;
            while (runner != dom_node->idom->basic_block) {
                dom_frontier[runner] = dom_node;
                runner = dom_map.at(runner)->idom->basic_block;
            }
        }
    }
    return dom_frontier;
}

Ir::Block *DomTree::LCA(Ir::Block *blk_1, Ir::Block *blk_2) {
    if (is_dom(blk_1, blk_2)) {
        return blk_2;
    }
    if (is_dom(blk_2, blk_1)) {
        return blk_1;
    }
    auto dom_1 = dom_set[blk_1];
    auto dom_2 = dom_set[blk_2];
    auto cur_dom = [this](Ir::Block *cur_blk) { return dom_map[cur_blk]; };
    if (dom_1.size() > dom_2.size()) {
        auto c = blk_1;
        blk_1 = blk_2;
        blk_2 = c;
    }
    dom_1 = dom_set[blk_1];
    dom_2 = dom_set[blk_2];
    while (dom_set[blk_2].size() > dom_1.size()) {
        blk_2 = cur_dom(blk_2)->idom->basic_block;
    }
    auto dom_node1 = cur_dom(blk_1).get();
    auto dom_node2 = cur_dom(blk_2).get();

    while (dom_node1 != dom_node2) {
        dom_node1 = dom_node1->idom;
        dom_node2 = dom_node2->idom;
    }
    return dom_node1->basic_block;
}

} // namespace Alys
