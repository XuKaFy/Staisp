#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cassert>
#include <cstddef>
#include <cstdio>

#include <functional>
#include <memory>
#include <string>

namespace Alys {

pDomBlock DomTree::make_domblk() { return std::make_shared<DomBlock>(); }

auto DomTree::build_dfsn(Set<DomBlock *> &v, DomBlock *cur) -> void {
    my_assert(!v.count(cur), "tree");
    v.insert(cur);
    dom_order.push_back(cur->basic_block.get());
    for (auto *out : cur->out_block) {
        build_dfsn(v, out);
    }
}

void DomTree::build_dom(Ir::BlockedProgram &p) {
    Map<Ir::Block *, int> order_map;

    for (const auto &bb : p.blocks) {
        auto [it, success] = dom_map.insert({bb.get(), make_domblk()});
        auto dom_it = it->second;
        my_assert(success, "insertion success");
        dom_it->basic_block = bb;
        order_map.insert({bb.get(), 0});
    }

    int level = 1;
    Vector<Ir::Block *> idfn;
    std::function<void(Ir::Block *, int &)> dfsn =
        [&dfsn, &idfn, &order_map](Ir::Block *bb, int &level) -> void {
        if (order_map.at(bb) == level) {
            return;
        }
        if (level == 1) {
            idfn.push_back(bb);
        }
        order_map.at(bb) = level;
        for (Ir::Block *succ : bb->out_blocks()) {
            dfsn(succ, level);
        }
    };

    auto *entry = p.blocks.front().get();
    dfsn(entry, level);
    for (auto *node : idfn) {
        level++;
        order_map.at(node) = level;
        my_assert(level != 1, "idfn will not changed");
        dfsn(entry, level);
        for (auto &[bb, dom_bb] : dom_map) {
            if (order_map.at(bb) != level) {
                dom_bb->idom = dom_map.at(node).get();
            }
        }
    }

    for (auto &[bb, dom_bb] : dom_map) {
        if (dom_bb->idom != nullptr) {
            dom_bb->idom->out_block.push_back(dom_bb.get());
        }
    }
    Set<DomBlock *> v;
    build_dfsn(v, dom_map[entry].get());
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
            my_assert(dom_map.at(j->basic_block.get())->idom == dom_node.get(),
                      "dominance tree success");
        }
        puts("\n\n");
    }
};

Map<Ir::Block *, pDomBlock> DomTree::build_dom_frontier() const {
    Map<Ir::Block *, pDomBlock> dom_frontier;
    for (const auto &[bb, dom_node] : dom_map) {
        for (auto *in : bb->in_blocks()) {
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

PhiInstr::PhiInstr(const pType &type)
    : Instr(is_pointer(type) ? to_pointed_type(type) : type) {};

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

    ret += "phi " + ty->type_name() + " ";
    for (size_t index = 0; index < operands.size(); ++index) {
        auto val = operands.at(index);
        auto *lab_use = labels.at(index);
        ret += "[ ";
        ret += val->usee->name();
        ret += ", %";
        ret += lab_use->name();
        ret += " ], ";
    }

    ret.pop_back();
    ret.pop_back();
    return ret;
}

void PhiInstr::add_incoming(Block *blk, Val *val) {
    my_assert(is_same_type(val->ty, ty), "operand is same as type of phi node");
    add_operand(val);
    add_label(blk);
}

void PhiInstr::add_label(Block *blk) {
    auto blk_label = blk->label();
    labels.push_back(blk_label.get());
}

pInstr make_phi_instr(const pType &type) {
    return std::make_shared<PhiInstr>(type);
}

}; // namespace Ir
