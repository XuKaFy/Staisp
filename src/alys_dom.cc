#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include <cassert>
#include <cstdio>

namespace Alys {

pDomBlock DomTree::make_domblk() { return pDomBlock(new DomBlock()); }

void DomTree::build_dom(Ir::BlockedProgram &p) {
    Map<Ir::Block *, int> order_map;

    for (auto bb : p.blocks) {
        auto [it, success] = dom_map.insert({bb.get(), make_domblk()});
        auto dom_it = it->second;
        my_assert(success, "insertion success");
        dom_it->basic_block = bb;
        order_map.insert({bb.get(), 0});
    }

    int level = 1;
    Vector<Ir::Block *> idfn;
    std::function<void(Ir::Block *)> dfsn =
        [&dfsn, &idfn, level, &order_map](Ir::Block *bb) -> void {
        if (order_map.at(bb) == level)
            return;
        if (level == 1)
            idfn.push_back(bb);
        order_map.at(bb) = level;
        for (Ir::Block *succ : bb->out_block) {
            dfsn(succ);
        }
    };

    auto entry = p.blocks.front().get();
    dfsn(entry);
    for (auto node : idfn) {
        level++;
        order_map.at(node) = level;
        my_assert(level != 1, "idfn will not changed");
        dfsn(entry);
        for (auto [bb, dom_bb] : dom_map) {
            if (order_map.at(bb) != level) {
                dom_bb->idom = dom_map.at(node).get();
                dom_map.at(node)->out_block.insert(dom_bb.get());
            }
        }
    }
    return;
}

void DomTree::print_dom_tree() const {
    for (auto [_, dom_node] : dom_map) {
        /*
        printf("CFG: Block %s\n", i->name());
        for (auto j : i->in_block) {
            printf("    In Block %s\n", j->name());
        }
        for (auto j : i->out_block) {
            printf("    Out Block %s\n", j->name());
        }
        */
        printf("Dominance Tree: Block %s\n", dom_node->basic_block->name());
        printf("Idom: %s", dom_node->idom->basic_block->name());
        printf("Out Block: ");
        for (auto j : dom_node->out_block) {
            printf("%s ;", j->basic_block->name());
            my_assert(dom_map.at(j->basic_block.get())->idom == dom_node.get(),
                      "dominance tree success");
        }
        putchar('\n');
    }
};

Map<Ir::Block *, pDomBlock> DomTree::build_dom_frontier() const {
    Map<Ir::Block *, pDomBlock> dom_frontier;
    for (auto [bb, dom_node] : dom_map) {
        for (auto in : bb->in_block) {
            Ir::Block *runner = in;
            while (runner != dom_node->idom->basic_block.get()) {
                dom_frontier[runner] = dom_node;
                runner = dom_map.at(runner)->idom->basic_block.get();
            }
        }
    }
    return dom_frontier;
}

} // namespace Alys