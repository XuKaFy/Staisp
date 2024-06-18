#include "alys_dom.h"
#include "ir_block.h"
#include "ir_control_instr.h"
#include "ir_opr_instr.h"
#include "ir_phi_instr.h"

namespace Ir {

void BlockedProgram::opt_join_blocks() {

    for (auto i = blocks.begin(); i != blocks.end();) {
        /*
        \     |     /
         [next_block]
              |
         [cur_block]
          /   |   \
        */
        auto cur_block = i->get();
        auto in_blocks = cur_block->in_blocks();
        if (in_blocks.size() != 1) {
            ++i;
            continue;
        }

        auto next_block = *in_blocks.begin();
        if (next_block->out_blocks().size() != 1) {
            ++i;
            continue;
        }

        /*
        printf("Block {\n%s} connected with {\n%s}\n",
        next_block->print_block().c_str(), cur_block->print_block().c_str());
        printf("Block both connected\n\n");
        */

        /*
        printf("Label %s -> %s\n", cur_block->label()->name().c_str(),
                                    next_block->label()->name().c_str());
        */

        next_block->pop_back();

        for (auto j = ++cur_block->begin(); j != cur_block->end();
             ++j) {
            next_block->push_back(std::move(*j));
        }

        // printf("For Instr: %s\n", next_block->label()->name().c_str());
        for (auto i : next_block->label()->users) {
            /*if (!i) {
                printf("    there's empty Use\n");
                continue;
            } else {
                printf("    User: %s\n", dynamic_cast<Instr*>(i->user)->instr_print().c_str());
            }*/
            Ir::PhiInstr *phi = dynamic_cast<PhiInstr*>(i->user);
            if (phi && phi->block() == next_block) {
                for (size_t i=0; i<phi->operand_size()/2; ++i) {
                    if (phi->phi_label(i) == next_block->label().get()) {
                        phi->replace_self(phi->phi_val(i));
                        next_block->erase(phi);
                        break;
                    }
                }
            }
        }

        cur_block->label()->replace_self(next_block->label().get());

        i = blocks.erase(i);
    }
}

void BlockedProgram::opt_remove_unreachable_block() {
    my_assert(!blocks.empty(), "?");
    Set<Block*> visitedBlock;
    Queue<Block*> q;
    q.push(blocks.front().get());
    while (!q.empty()) {
        Block* cur = q.front();
        q.pop();
        for (auto i : cur->out_blocks()) {
            if (!visitedBlock.count(i)) {
                visitedBlock.insert(i);
                q.push(i);
            }
        }
    }
    for (auto i = ++begin(); i != end();) {
        if (!visitedBlock.count(i->get())) {
            // printf("Block {\n%s} removed\n", (*i)->print_block().c_str());
            i = erase(i);
        } else {
            ++i;
        }
    }
}

void BlockedProgram::opt_remove_only_jump_block() {
    my_assert(!empty(), "?");
    for (auto i = ++begin(); i != end();) {
        if ((*i)->out_blocks().size() == 1 && (*i)->size() == 2) {
            bool can_remove = true;
            for (auto j : (*i)->label()->users) {
                if (dynamic_cast<Ir::PhiInstr*>(j->user)) {
                    can_remove = false;
                    break;
                }
            }
            if (!can_remove) {
                ++i;
                continue;
            }
            // printf("Block {\n%s} connected\n", (*i)->print_block().c_str());
            (*i)->connect_in_and_out();
            i = erase(i);
        } else {
            ++i;
        }
    }
}

bool can_be_removed(Ir::InstrType t) {
    switch (t) {
    case Ir::INSTR_LABEL:
    case Ir::INSTR_RET:
    case Ir::INSTR_BR:
    case Ir::INSTR_BR_COND:
    case Ir::INSTR_STORE:
    case Ir::INSTR_CALL:
    case Ir::INSTR_UNREACHABLE:
    case Ir::INSTR_PHI:
        return false;
    default:
        return true;
    }
    return true;
}

void BlockedProgram::opt_remove_dead_code() {
    my_assert(!blocks.empty(), "?");
    for (const auto &block : blocks) {
        for (auto it = --block->end(); it != block->begin(); --it) {
            if ((*it)->users.empty() && can_be_removed((*it)->instr_type())) {
                it = block->erase(it);
            }
        }
    }
}

void BlockedProgram::opt_simplify_branch()
{
    for (const auto &i : blocks) {
        if (i->size() <= 1) {
            continue;
        }

        auto end = i->back();
        if (end->instr_type() != Ir::INSTR_BR_COND) {
            continue;
        }

        auto cond = end->operand(0)->usee;
        if (cond->type() != Ir::VAL_CONST) {
            continue;
        }

        auto con = static_cast<Ir::Const *>(cond);
        if (con->v.type() == VALUE_IMM) {
            i->squeeze_out((bool)con->v);
        }
    }
}

std::optional<Value> extractConstant(BinInstrType type, Val* val) {
    if (auto bin = dynamic_cast<Ir::BinInstr*>(val)) {
        auto rhs = bin->operand(1)->usee;
        if (bin->binType == type && rhs->type() == VAL_CONST) {
            return static_cast<Const *>(rhs)->v;
        }
    }
    return std::nullopt;
}

void optimize_divide_after_multiply(const pInstr &instr) {
    if (auto value1 = extractConstant(INSTR_SDIV, instr.get())) {
        auto lhs = instr->operand(0)->usee;
        if (auto value2 = extractConstant(INSTR_MUL, lhs); value1 == value2) {
            auto original = static_cast<Instr*>(lhs)->operand(0)->usee;
            instr->replace_self(original);
        }
    }
}
void optimize_subtract_after_add(const pInstr &instr) {
    if (auto value1 = extractConstant(INSTR_SUB, instr.get())) {
        auto lhs = instr->operand(0)->usee;
        if (auto value2 = extractConstant(INSTR_ADD, lhs); value1 == value2) {
            auto original = static_cast<Instr*>(lhs)->operand(0)->usee;
            instr->replace_self(original);
        }
    }
}


void optimize_multiply_one(const pInstr &instr) {
    if (auto value1 = extractConstant(INSTR_MUL, instr.get())) {
        auto lhs = instr->operand(0)->usee;
        if (value1 == Value(ImmValue(1))) {
            instr->replace_self(lhs);
        }
    }
}

// accumulate b + a + ... + a => b + k*a
void optimize_accumulate(const pBlock &block) {
    std::unordered_set<Val*> merged;
    for (auto it = block->begin(); it != block->end(); ++it) {
        if (merged.count(it->get())) continue;
        if (auto bin = dynamic_cast<Ir::BinInstr*>(it->get())) {
            auto lhs = bin->operand(0)->usee;
            auto rhs0 = bin->operand(1)->usee;
            auto acc = bin;
            int cnt = 0;
            while (bin && bin->binType == INSTR_ADD && bin->users.size() == 1) {
                auto rhs = bin->operand(1)->usee;
                if (rhs != rhs0) break;
                ++cnt;
                merged.insert(bin);
                acc = bin;
                bin = dynamic_cast<Ir::BinInstr*>(bin->users[0]->user);
            }
            if (cnt <= 1) continue;
            if (lhs == rhs0) {
                ++cnt;
                auto multiply = std::make_shared<BinInstr>(INSTR_MUL, block->add_imm(ImmValue(cnt)).get(), rhs0);
                it = block->insert(it, multiply);
                acc->replace_self(multiply.get());
            } else {
                auto multiply = std::make_shared<BinInstr>(INSTR_MUL, block->add_imm(ImmValue(cnt)).get(), rhs0);
                auto add = std::make_shared<BinInstr>(INSTR_ADD, lhs, multiply.get());
                it = block->insert(it, add);
                it = block->insert(it, multiply);
                acc->replace_self(add.get());
            }
        }
    }
}


void BlockedProgram::opt_trivial() {
    for (const auto &block : blocks) {
        for (const auto &instr : block->body) {
            optimize_divide_after_multiply(instr);
            optimize_multiply_one(instr);
            optimize_subtract_after_add(instr);
        }
        optimize_accumulate(block);
    }
}

}