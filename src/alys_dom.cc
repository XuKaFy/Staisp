#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cassert>
#include <cstdio>

#include <functional>
#include <memory>

namespace Alys {

pDomBlock DomTree::make_domblk() { return std::make_shared<DomBlock>(); }

void DomTree::build_dom(Ir::BlockedProgram &p) {
    Map<Ir::Block *, int> order_map;

    for (const auto& bb : p.blocks) {
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
        if (order_map.at(bb) == level) {
            return;
}
        if (level == 1) {
            idfn.push_back(bb);
}
        order_map.at(bb) = level;
        for (Ir::Block *succ : bb->out_blocks()) {
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
    }

void DomTree::print_dom_tree() const {
    for (auto [_, dom_node] : dom_map) {
        printf("Dominance Tree: Block %s\n",
               dom_node->basic_block->name().c_str());
        printf("Idom: %s", dom_node->idom->basic_block->name().c_str());
        printf("Out Block: ");
        for (auto j : dom_node->out_block) {
            printf("%s ;", j->basic_block->name().c_str());
            my_assert(dom_map.at(j->basic_block.get())->idom == dom_node.get(),
                      "dominance tree success");
        }
        putchar('\n');
    }
};

Map<Ir::Block *, pDomBlock> DomTree::build_dom_frontier() const {
    Map<Ir::Block *, pDomBlock> dom_frontier;
    for (auto [bb, dom_node] : dom_map) {
        for (auto in : bb->in_blocks()) {
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

namespace Ir {
String PhiInstr::instr_print() const {
    /*
    Phi syntax:
    %indvar = phi i32 [ 0, %LoopHeader ], [ %nextindvar, %Loop ]
    <result> = phi [fast-math-flags] <ty> [ <val0>, <label0>],
    */
    String ret;
    my_assert(ty->type_type() != TYPE_VOID_TYPE,
              "Phi Instruction type must be non-void");
    ret = name() + " = ";

    ret += "phi " + ty->type_name();
    for (auto [blk, val] : incoming_tuples) {
        ret += "[ ";
        ret += val->usee->name();
        ret += ", ";
        ret += blk->name();
        ret += " ]";
    }

    return ret;
}

void PhiInstr::add_incoming(Block *blk, Val *val) {
    my_assert(val->ty == ty, "operand is same as type of phi node");
    my_assert(!incoming_tuples.count(blk), "block is not in incoming tuples");
    add_operand(val);
    incoming_tuples.insert({blk, *operands.cbegin()});
}

pInstr make_phi_instr(const pType& type) { return pInstr(new PhiInstr(type)); }

}; // namespace Ir