#include "ir_block.h"
#include "ir_opr_instr.h"

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

        // printf("Block {\n%s} connected with {\n%s}\n",
        // next_block->print_block().c_str(), cur_block->print_block().c_str());
        // printf("Block both connected\n\n");

        next_block->pop_back();
        for (auto j = ++cur_block->begin(); j != cur_block->end();
             ++j) {
            next_block->push_back(std::move(*j));
        }

        i = blocks.erase(i);
    }
}

void BlockedProgram::opt_remove_empty_block() {
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

void BlockedProgram::opt_connect_empty_block() {
    my_assert(!empty(), "?");
    for (auto i = ++begin(); i != end();) {
        if ((*i)->out_blocks().size() == 1 && (*i)->size() == 2) {
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
    for (const auto &i : blocks) {
        for (auto j = ++i->begin(); j != i->end();) {
            if ((*j)->users.empty() && can_be_removed((*j)->instr_type())) {
                j = i->erase(j);
            } else {
                ++j;
            }
        }
    }

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

void BlockedProgram::opt_trivial() {
    for (const auto &block : blocks) {
        for (const auto &instr : block->body) {
            optimize_divide_after_multiply(instr);
        }
        // accumulate a + a + ... + a => k*a
        for (auto it = block->begin(); it != block->end(); ++it) {
            if (auto bin = dynamic_cast<Ir::BinInstr*>(it->get())) {
                auto lhs = bin->operand(0)->usee;
                auto rhs = bin->operand(1)->usee;
                if (bin->binType == INSTR_ADD && lhs == rhs && bin->users.size() == 1) {
                    // start to accumulate
                    int cnt = 2;
                    auto old_acc = bin;
                    auto acc = bin->users[0]->user;
                    while ((bin = dynamic_cast<Ir::BinInstr*>(acc))) {
                        rhs = bin->operand(1)->usee;
                        if (bin->binType == INSTR_ADD && lhs == rhs && bin->users.size() == 1) {
                            ++cnt;
                            old_acc = bin;
                            acc = bin->users[0]->user;
                        } else {
                            break;
                        }
                    }
                    auto inserted = std::make_shared<BinInstr>(INSTR_MUL, lhs, add_imm(ImmValue(cnt)).get());
                    it = block->insert(it, inserted);
                    old_acc->replace_self(inserted.get());
                }
            }
        }
    }
}

}