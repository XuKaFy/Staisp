#include "trans_SSA.h"
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"
#include "type.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <unordered_set>
/*
This algorithem is based on the paper "Simple and Efficient Construction of
Static Single Assignment Form" by Braun, M., Buchwald, S., Hack, S., Leißa, R.,
Mallon, C., & Zwinkau, A. (2013).
https:doi.org/10.1007/978-3-642-37051-9_6
*/

namespace Optimize {

SSA_pass::SSA_pass(Ir::BlockedProgram &arg_function,
                   const ssa_type &arg_ssa_type)
    : promotion_type(arg_ssa_type), cur_func(arg_function) {
    dom_ctx.build_dom(arg_function);
#ifdef dom_bug
    dom_ctx.print_dom();
#endif
    arg_function.normal_opt();
#ifdef ssa_debug
    printf("Before SSA transformation:\n");
    cur_func.print();
#endif
}

auto SSA_pass::entry_blk() -> Ir::Block * {
    return cur_func.blocks.at(0).get();
}

// the defintion of $variable in $block is $blk_def_val
auto SSA_pass::def_val(vrtl_reg *variable, Ir::Block *block,
                       vrtl_reg *blk_def_val) -> void {
    current_def[variable][block] = blk_def_val;
}

auto SSA_pass::is_phi(Ir::User *user) -> bool {
    if (user->type() == Ir::VAL_INSTR) {
        auto *cur_instr = dynamic_cast<Ir::Instr *>(user);
        return my_assert(cur_instr, "must be Instr *"),
               cur_instr->instr_type() == Ir::INSTR_PHI;
    }
    return false;
}

SSA_pass::vrtl_reg *SSA_pass::use_val(vrtl_reg *variable, Ir::Block *block) {
    if (current_def.at(variable).count(block) > 0) {
        return current_def[variable][block];
    }
    my_assert(block != entry_blk(),
              "The definition of every variable must be in the entry block.");
    return use_val_recursive(variable, block);
}

SSA_pass::vrtl_reg *SSA_pass::use_val_recursive(vrtl_reg *variable,
                                                Ir::Block *block) {
    vrtl_reg *val = nullptr;
    if (sealedBlocks.find(block) == sealedBlocks.end()) {
        // Incomplete CFG.
        auto phi = Ir::make_phi_instr(variable->ty);
        phi->block = block;
        val = phi.get();
        block->body.insert(std::next(block->body.begin()), phi);
        // my_assert(block->body.at(1) == phi, "head insertion");
        incompletePhis[block].emplace_back(variable, phi.get());
    } else if (block->in_blocks().size() == 1) {
        // Optimize the common case of one predecessor: No phi needed
        val = use_val(variable, *block->in_blocks().begin());
    } else {
        my_assert(!block->in_blocks().empty(),
                  "every block in such function must have predecessor");
        // Break potential cycles with operandless phi
        auto phi = Ir::make_phi_instr(variable->ty);
        phi->block = block;
        block->body.insert(std::next(block->body.begin()), phi);
        // my_assert(block->body.at(1) == phi, "head insertion");
        def_val(variable, block, phi.get());
        val = addPhiOperands(variable, phi.get(), block);
    }
    def_val(variable, block, val);
    return val;
}

SSA_pass::vrtl_reg *SSA_pass::addPhiOperands(vrtl_reg *variable, Ir::Instr *phi,
                                             Ir::Block *phi_blk) {
    auto *phi_ins = dynamic_cast<Ir::PhiInstr *>(phi);
    for (auto *pred : phi_blk->in_blocks()) {
        auto *pred_val = use_val(variable, pred);
        // phi->add_incoming(pred, val);
        phi_ins->add_incoming(pred, pred_val);
    }
    return tryRemoveTrivialPhi(phi_ins);
}

auto SSA_pass::tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg * {

    vrtl_reg *same = nullptr;

    auto is_trivial = [&same, &phi]() -> bool {
        std::unordered_set<Ir::pUse> trivial_phi_operands{phi->operands.begin(),
                                                          phi->operands.end()};
        if (trivial_phi_operands.size() >= 3) {
            return false;
        }

        if (trivial_phi_operands.size() == 1) {
            auto src_val = phi->operands.at(0);
            my_assert(src_val->user == phi, "semantics of phi USE");
            same = src_val->usee;
            return true;
        }

        auto op1 = *trivial_phi_operands.begin();
        auto op2 = *std::next(trivial_phi_operands.begin());
        if (op1->usee == phi) {
            same = op2->usee;
            return true;
        }
        if (op2->usee == phi) {
            same = op1->usee;
            return true;
        }
        my_assert(op1->usee != phi && op2->usee != phi, "no self-loop phi");
        return false;
    };

    if (!is_trivial()) {
        return phi;
    }
    my_assert(same, "nullptr to some val in phi incoming tuples");
    Vector<Ir::PhiInstr *> phiUsers;

    for (Ir::User *user : fmap<Ir::pUse, Ir::User *>(
             [&phi](auto arg_use) {
                 my_assert(arg_use->usee == phi,
                           "the source of such use-def edge must be phi instr");
                 return arg_use->user;
             },
             phi->users)) {
        if (user != phi && is_phi(user)) {
            phiUsers.push_back(static_cast<Ir::PhiInstr *>(user));
        }
    }

    // reset val
    phi->replace_self(same);

    auto remove_trivial = [&phi]() {
        my_assert(phi->users.empty(), "successful replace");
        for (auto ins_it = ++phi->block->body.begin();
             ins_it != phi->block->body.end(); ins_it++) {
            if ((*ins_it).get() == phi) {
                phi->block->body.erase(ins_it);
                break;
            }
        }
    };
    remove_trivial();
    for (auto *user : phiUsers) {
        tryRemoveTrivialPhi(user);
    }
    return same;
}

void SSA_pass::sealBlock(Ir::Block *block) {
    if (incompletePhis.find(block) != incompletePhis.end()) {
        for (auto [variable, phi] : incompletePhis[block]) {
            addPhiOperands(variable, phi, block);
        }
        incompletePhis.erase(block);
    }
    sealedBlocks.insert(block);
}

auto SSA_pass::unreachable_blks() -> Set<Ir::Block *> {
    Set<Ir::Block *> unreachable;
    for (const auto &bb : cur_func.blocks) {
        if (dom_ctx.dom_map.count(bb.get()) == 0) {
            // unreachable block
            unreachable.insert(bb.get());
        }
    }
    return unreachable;
}

void SSA_pass::reconstruct() {
    my_assert(unreachable_blks().empty(), "no unreachable blocks");

    auto const promotable_filter = [](Ir::Val *arg_alloca_instr) -> bool {
        return !is_array(arg_alloca_instr->ty) &&
               !is_struct(arg_alloca_instr->ty) &&
               !(arg_alloca_instr->type() == Ir::VAL_GLOBAL) &&
               (arg_alloca_instr->ty->length() <= 4 ||
                is_pointer(arg_alloca_instr->ty));
    };

    // static pointer or run time initialized pointer
    auto const pointer_verifier =
        [&promotable_filter](Ir::Val *arg_non_alloca) -> bool {
        return !promotable_filter(arg_non_alloca) ||
               (is_pointer(arg_non_alloca->ty) &&
                dynamic_cast<Ir::ItemInstr *>(arg_non_alloca));
    };

    Set<vrtl_reg *> alloca_vars;
    auto build_def = [this, &alloca_vars, &promotable_filter]() -> void {
        for (const auto &ent_instr : entry_blk()->body) {
            if (ent_instr->instr_type() == Ir::INSTR_ALLOCA) {
                auto *alloca = dynamic_cast<Ir::AllocInstr *>(ent_instr.get());
                if (promotable_filter(alloca)) {
                    alloca_vars.insert(alloca);
                    current_def[alloca] = {};
                }
            }
        }
    };
    if (build_def(); alloca_vars.empty()) {
        return;
    }

    Map<Ir::Block *, size_t> pred = {};
    std::transform(cur_func.blocks.begin(), cur_func.blocks.end(),
                   std::inserter(pred, pred.end()), [](const auto &bb) {
                       return std::make_pair(bb.get(), bb->in_blocks().size());
                   });
    auto trySeal = [this, &pred](Ir::Block *block) -> void {
        if (sealedBlocks.count(block) == 0 && pred[block] == 0) {
            sealBlock(block);
        };
    };

    auto mem_destination_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operands.at(0);
    };

    auto mem_sourve_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operands.at(1);
    };

    Vector<Ir::Block *> order;
    order.insert(order.begin(), dom_ctx.order().begin(), dom_ctx.order().end());

    for (auto *cur_block : order) {
        auto it = cur_block->body.begin();
        my_assert((*it)->instr_type() == Ir::INSTR_LABEL, "label begin"), it++;
        for (; it != cur_block->body.end();
             /* next perform increment*/) {
            auto succ_instr = std::next(it);
            auto cur_instr = *it;
            if (cur_instr->instr_type() == Ir::INSTR_STORE) {
                auto *store = dynamic_cast<Ir::StoreInstr *>(cur_instr.get());
                const auto &to = mem_destination_val(store);
                if (alloca_vars.count(to->usee) > 0) {
                    auto val = mem_sourve_val(store);
                    def_val(to->usee, cur_block, val->usee);
                    cur_block->body.erase(it);
                } else {
                    my_assert(pointer_verifier(to->usee),
                              "store to non-alloca variable");
                }

            } else if (cur_instr->instr_type() == Ir::INSTR_LOAD) {
                auto *load = dynamic_cast<Ir::LoadInstr *>(cur_instr.get());
                auto src_ptr = load->operands.at(0);
                if (alloca_vars.count(src_ptr->usee) > 0) {
                    load->replace_self(use_val(src_ptr->usee, cur_block));
                    my_assert(load->users.empty(), "removable load instr");
                    cur_block->body.erase(it);
                } else {
                    my_assert(pointer_verifier(src_ptr->usee),
                              "load from non-alloca variable");
                }
            }
            it = succ_instr;
        }
        trySeal(cur_block);
        for (auto *succ_bb : cur_block->out_blocks()) {
            pred[succ_bb]--;
            trySeal(succ_bb);
        }
    }

    my_assert(sealedBlocks.size() == cur_func.blocks.size(),
              "all blocks are sealed");

    for (auto ent_instr_it = ++entry_blk()->body.begin();
         ent_instr_it != entry_blk()->body.end();) {
        if (auto *ent_instr =
                dynamic_cast<Ir::AllocInstr *>(ent_instr_it->get());
            ent_instr != nullptr) {
            if (promotable_filter(ent_instr)) {
                // my_assert(ent_instr->users.empty(), "single value is
                // removable");
                if (ent_instr->users.empty()) {
                    ent_instr_it = entry_blk()->body.erase(ent_instr_it);
                    continue;
                }
            }
        }
        ent_instr_it++;
    }
}

void SSA_pass::pass_transform() {
    my_assert(promotion_type == ssa_type::RECONSTRUCTION, "RECONS");
    reconstruct();
}

} // namespace Optimize