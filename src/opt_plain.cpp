#include <ir_call_instr.h>
#include <ir_cast_instr.h>
#include <ir_func_defined.h>
#include <ir_ptr_instr.h>
#include <iterator>
#include <optional>

#include "alys_dom.h"
#include "ir_block.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "type.h"

namespace Ir {

bool BlockedProgram::is_pure() const
{
    // if (function_type()->ret_type->type_type() == TYPE_VOID_TYPE || function_type()->arg_type.empty()) return false;
    for (auto&& arg_type : params_) {
        if (!is_basic_type(arg_type->ty)) return false;
    }
    for (auto i = cbegin(); i != cend(); ++i) {
        auto block = *i;
        for (auto&& instr : *block) {
            switch (instr->instr_type()) {
                case INSTR_CALL: {
                    auto call = static_cast<CallInstr*>(instr.get());
                    auto func = call->operand(0)->usee;
                    auto callee = dynamic_cast<FuncDefined*>(func);
                    if (!callee)
                        return false;
                    if (callee->p.name() != name() && !callee->p.is_pure()) {
                        // recursion do not cancel purity
                        return false;
                    }
                    break;
                }
                case INSTR_LOAD:
                case INSTR_ITEM:
                case INSTR_STORE:
                case INSTR_CAST:
                case INSTR_MINI_GEP: {
                    if (instr->operand(0)->usee->name()[0] == '@') {
                        return false;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return true;
}

void BlockedProgram::plain_opt_bb() {
    int modified = 1;
    while (modified) {
            modified = 0;
        // 连接块不需要考虑 PHI
        // 因为所有的 PHI 在无分支的情况下
        // 会替换自身为某个量
        modified += opt_join_blocks();
        my_assert(check_empty_use("JOIN BLOCK") == 0, "JOIN BLOCK FAILED");
        my_assert(check_invalid_phi("JOIN BLOCK") == 0, "JOIN BLOCK FAILED");
        // 简化分支需要考虑 PHI
        // 当某个分支被裁剪
        // 相应的 PHI 也要删掉入边
        // 此处对 PHI 的修改是 release_operand
        modified += opt_simplify_branch();
        my_assert(check_empty_use("SIMPLIFY BRANCH") == 0, "SIMPLIFY BRANCH FAILED");
        my_assert(check_invalid_phi("SIMPLIFY BRANCH") == 0, "SIMPLIFY BRANCH FAILED");
        // 删掉所有 if (xxx) N=1 else N=0 的分支
        modified += opt_remove_simple_branch();
        my_assert(check_empty_use("REMOVE SIMPLE BRANCH") == 0, "REMOVE SIMPLE BRANCH");
        my_assert(check_invalid_phi("REMOVE SIMPLE BRANCH") == 0, "REMOVE SIMPLE BRANCH");
        // 删除块需要考虑 PHI
        // PHI 指令可能存在某些入边
        // 来自于永远不会执行的块
        // 此时对 PHI 的修改体现在 erase_from_phi
        modified += opt_remove_unreachable_block();
        my_assert(check_empty_use("REMOVE UNREACHABLE") == 0, "REMOVE UNREACHABLE FAILED");
        my_assert(check_invalid_phi("REMOVE UNREACHABLE") == 0, "REMOVE UNREACHABLE FAILED");
        // 简化简单块也需要考虑 PHI
        // 考虑以下情况：
        // L1: jmp L2
        // L2: jmp L3
        // L3: phi [1, L2], ...
        // PHI 指令需要被修改
        // 注意，若是以下情况：
        // L1: if ... jmp L2 else jmp L3
        // L2: jmp L3
        // L3: phi [1, L2], [2, L1]
        // 这个块就不能被删掉
        // 此处对 PHI 的修改体现在 change_operand 和 erase_from_phi
        modified += opt_remove_only_jump_block();
        my_assert(check_empty_use("REMOVE ONLY JUMP") == 0, "REMOVE ONLY JUMP FAILED");
        my_assert(check_invalid_phi("REMOVE ONLY JUMP") == 0, "REMOVE ONLY JUMP FAILED");
        plain_opt_no_bb();
    }
}

void BlockedProgram::plain_opt_no_bb() {
    // DO NOT EDIT: TWICE AS INTENDED
    // opt_trivial();
    opt_trivial();
    opt_remove_dead_code();
}

void BlockedProgram::plain_opt_all() {
    plain_opt_bb();
}

struct SingleAssign {
    Ir::Val* to;
    Ir::Block* next_block;
    int x;
};

std::optional<SingleAssign> check_block(Ir::Block* block)
{
    if (block->size() != 3)
        return {};
    
    SingleAssign ass;
    
    if (block->back()->instr_type() != Ir::INSTR_BR)
        return {};
    ass.next_block = static_cast<Ir::LabelInstr*>(block->back()->operand(0)->usee)->block();

    auto store = dynamic_cast<Ir::StoreInstr*>(std::next(block->begin())->get());
    if (store == nullptr)
        return {};
    ass.to = store->operand(0)->usee;

    auto imm = dynamic_cast<Ir::Const*>(store->operand(1)->usee);
    if (imm == nullptr || !is_basic_type(imm->ty))
        return {};
    ass.x = imm->v.imm_value().val.ival;

    return ass;
}

bool BlockedProgram::opt_remove_simple_branch()
{
    // re_generate();
    auto try_remove = [](Ir::Block* block) -> bool {
        if (block->back()->instr_type() != INSTR_BR_COND)
            return false;
        auto br_cond = std::static_pointer_cast<Ir::BrCondInstr>(block->back());
        auto true_block = static_cast<Ir::LabelInstr*>(br_cond->operand(1)->usee)->block();
        auto false_block = static_cast<Ir::LabelInstr*>(br_cond->operand(2)->usee)->block();
        auto true_block_info = check_block(true_block);
        auto false_block_info = check_block(false_block);
        if (!true_block_info || !false_block_info)
            return false;
        if (true_block_info->next_block != false_block_info->next_block)
            return false;
        if (true_block_info->to != false_block_info->to)
            return false;
        if (true_block_info->x != 1 || false_block_info->x != 0)
            return false;
        // printf("FOUND BLOCK %s\n", block->print_block().c_str());
        // printf("TRUE BLOCK %s\n", true_block->print_block().c_str());
        // printf("FALSE BLOCK %s\n", false_block->print_block().c_str());
        // printf("TO BLOCK %s\n", true_block_info->next_block->print_block().c_str());
        block->pop_back();
        block->push_back(Ir::make_br_instr(true_block_info->next_block->label().get()));
        Ir::Val* value = br_cond->operand(0)->usee;
        if (!is_same_type(to_pointed_type(true_block_info->to->ty), value->ty)) {
            auto cast = Ir::make_cast_instr(to_pointed_type(true_block_info->to->ty), value);
            block->push_behind_end(cast);
            value = cast.get();
        }
        block->push_behind_end(Ir::make_store_instr(true_block_info->to, value));
        true_block->erase_from_phi();
        false_block->erase_from_phi();
        // printf("AFTER BLOCK %s\n", block->print_block().c_str());
        return true;
    };
    int ans = 0;
    for (auto i = begin(); i != end(); ++i) {
        if (try_remove(i->get())) {
            // printf("ONCE opt_remove_simple_branch\n\n");
            ++ans;
        }
    }
    return ans;
}

bool BlockedProgram::opt_join_blocks() {
    bool modified = false;

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

        next_block->pop_back();

        /*
        printf("REPLACE LABEL %s -> %s\n", cur_block->label()->name().c_str(),
            next_block->label()->name().c_str());
        */
        
        cur_block->label()->replace_self(next_block->label().get());
        
        for (auto j = ++cur_block->begin(); j != cur_block->end();
             ++j) {
            next_block->push_back(std::move(*j));
        }

        // printf("REMOVE BLOCK %s\n", cur_block->name().c_str());

        i = erase(i);
        modified = true;
    }
    return modified;
}

bool BlockedProgram::opt_remove_unreachable_block() {
    my_assert(!blocks.empty(), "?");
    bool modified = false;
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
    for (auto i = ++begin(); i != end(); ++i) {
        if (!visitedBlock.count(i->get())) {
            modified = true;
            (*i)->erase_from_phi();
        }
    }
    for (auto i = ++begin(); i != end();) {
        if (!visitedBlock.count(i->get())) {
            i = erase(i);
        } else {
            ++i;
        }
    }
    return modified;
}

bool BlockedProgram::opt_remove_only_jump_block() {
    my_assert(!empty(), "?");
    bool modified = false;

    for (auto i = ++begin(); i != end();) {
        if ((*i)->out_blocks().size() != 1 || (*i)->size() != 2) {
            ++i;
            continue;
        }
        bool can_be_removed = true;
        for (auto k : (*i)->in_blocks()) {
            if (k->back()->instr_type() == INSTR_BR_COND) {
                can_be_removed = false;
                break;
            }
        }
        if (!can_be_removed) {
            ++i;
            continue;
        }
        (*i)->connect_in_and_out();
        i = erase(i);
        modified = true;
    }
    return modified;
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

bool is_pure_function(const Ir::pInstr& instr) {
    auto call_instr = dynamic_cast<Ir::CallInstr *>(instr.get());
    auto callee = call_instr ? call_instr->operand(0)->usee : nullptr;
    auto callee_func = dynamic_cast<Ir::FuncDefined *>(callee);
    return callee_func && callee_func->is_pure();
}

void BlockedProgram::opt_remove_dead_code() {
    my_assert(!blocks.empty(), "?");
    for (const auto &block : blocks) {
        for (auto it = --block->end(); it != block->begin(); --it) {
            if ((*it)->users().empty() && (can_be_removed((*it)->instr_type()) || is_pure_function(*it))) {
                it = block->erase(it);
            }
        }
        bool terminated = false;
        for (auto it = block->begin(); it != block->end(); ) {
            if ((*it)->is_terminator()) {
                if (!terminated) {
                    terminated = true;
                } else {
                    it = block->erase(it);
                    continue;
                }
            } else if(terminated) {
                it = block->erase(it);
                continue;
            }
            ++it;
        }
    }
}

bool BlockedProgram::opt_simplify_branch()
{
    bool modified = false;

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

        modified = true;
    }
    return modified;
}

std::optional<Value> extractConstant(Val* val) {
    if (val->type() == VAL_CONST) {
        return static_cast<Const *>(val)->v;
    }
    return {};
}

std::optional<Value> extractConstant(BinInstrType type, Val* val) {
    if (auto bin = dynamic_cast<Ir::BinInstr*>(val)) {
        auto rhs = bin->operand(1)->usee;
        if (rhs->type() == VAL_CONST && bin->binType == type) {
            return static_cast<Const *>(rhs)->v;
        }
    }
    return std::nullopt;
}

std::optional<Value> extractConstant(const std::initializer_list<BinInstrType>& types, Val* val) {
    if (auto bin = dynamic_cast<Ir::BinInstr*>(val)) {
        auto rhs = bin->operand(1)->usee;
        if (rhs->type() == VAL_CONST && std::find(types.begin(), types.end(), bin->binType) != types.end()) {
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

// x * 1 => x
// x / 1 => x
// x + 0 => x
// x - 0 => x
// x | 0 => x
// x ^ 0 => x
void optimize_identity_operation(const pInstr &instr) {
    if (auto value1 = extractConstant({INSTR_MUL, INSTR_UDIV, INSTR_SDIV}, instr.get())) {
        auto lhs = instr->operand(0)->usee;
        if (value1 == Value(ImmValue(1))) {
            instr->replace_self(lhs);
        }
    }
    if (auto value1 = extractConstant({INSTR_ADD, INSTR_SUB, INSTR_OR, INSTR_XOR}, instr.get())) {
        auto lhs = instr->operand(0)->usee;
        if (value1 == Value(ImmValue(0))) {
            instr->replace_self(lhs);
        }
    }
}
// x * 0 => 0
// x & 0 => 0
void optimize_constant_propagate(const pInstr &instr) {
    if (auto value1 = extractConstant({INSTR_MUL, INSTR_AND}, instr.get())) {
        auto rhs = instr->operand(1)->usee;
        if (value1 == Value(ImmValue(0))) {
            instr->replace_self(rhs);
        }
    }
}

// for x, y of i1
// zext32(x) bitwise zext32(y) => zext32(x bitwise y)
void optimize_bitwise_boolean(const pBlock &block) {
    std::unordered_set<Val*> merged;
    for (auto it = block->begin(); it != block->end(); ++it) {
        if (merged.count(it->get())) continue;
        if (auto bin = dynamic_cast<BinInstr*>(it->get())) {
            if (bin->binType == INSTR_OR || bin->binType == INSTR_AND || bin->binType == INSTR_XOR) {
                auto lhs = bin->operand(0)->usee;
                auto rhs = bin->operand(1)->usee;
                if (auto cast1 = dynamic_cast<CastInstr*>(lhs); cast1 && cast1->method() == CAST_ZEXT) {
                    auto bool1 = cast1->operand(0)->usee;
                    if (auto cast2 = dynamic_cast<CastInstr*>(rhs); cast2 && cast2->method() == CAST_ZEXT) {
                        auto bool2 = cast2->operand(0)->usee;
                        if (is_same_type(cast1->ty, cast2->ty) && is_same_type(bool1->ty, bool2->ty)
                                && is_integer(bool1->ty) && to_basic_type(bool1->ty)->ty == IMM_I1
                                && is_integer(cast1->ty) && to_basic_type(cast1->ty)->ty == IMM_I32) {
                            auto bitwise = std::make_shared<BinInstr>(bin->binType, bool1, bool2);
                            auto cast = std::make_shared<CastInstr>(bin->ty, bitwise.get());
                            it = block->insert(it, cast);
                            it = block->insert(it, bitwise);
                            bin->replace_self(cast.get());
                            merged.insert(bin);
                        }
                    }
                }
            }
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
            while (bin && bin->binType == INSTR_ADD && bin->users().size() == 1) {
                auto rhs = bin->operand(1)->usee;
                if (rhs != rhs0) break;
                ++cnt;
                merged.insert(bin);
                acc = bin;
                bin = dynamic_cast<Ir::BinInstr*>(bin->users().front()->user);
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

void optimize_unrolled_gep(const pBlock &block, const pInstr &instr) {
    auto gep = dynamic_cast<MiniGepInstr*>(instr.get());
    if (!gep) return;
    if (!gep->in_this_dim) return;
    auto parent = dynamic_cast<MiniGepInstr*>(gep->operand(0)->usee);
    if (!parent) return;
    if (!parent->in_this_dim) return;
    if (parent->block() != block.get()) return;
    auto grandparent = parent->operand(0)->usee;
    auto offset1 = extractConstant(gep->operand(1)->usee);
    if (!offset1) return;
    auto offset2 = extractConstant(parent->operand(1)->usee);
    if (!offset2) return;
    auto offset = offset1->imm_value().val.ival + offset2->imm_value().val.ival;
    gep->change_operand(0, grandparent);
    gep->change_operand(1, block->add_imm(Value(ImmValue(offset))).get());
}

void BlockedProgram::opt_trivial() {
    for (const auto &block : blocks) {
        for (const auto &instr : block->body) {
            optimize_divide_after_multiply(instr);
            optimize_identity_operation(instr);
            optimize_constant_propagate(instr);
            optimize_subtract_after_add(instr);
            optimize_unrolled_gep(block, instr);
        }
        optimize_bitwise_boolean(block);
        optimize_accumulate(block);
    }
}

}