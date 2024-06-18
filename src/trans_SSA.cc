#include "trans_SSA.h"
#include "alys_dom.h"
#include "def.h"
#include "imm.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_ptr_instr.h"
#include "ir_phi_instr.h"
#include "ir_val.h"
#include "type.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
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
    arg_function.plain_opt_all();
    dom_ctx.build_dom(arg_function);
#ifdef ssa_debug
    dom_ctx.print_dom_tree();
#endif

    auto undefined_initializer = [this]() {
        undefined_values.i32 = entry_blk()->add_imm(ImmValue(ImmType::IMM_I32));
        undefined_values.f32 = entry_blk()->add_imm(ImmValue(ImmType::IMM_F32));
        undefined_values.i1 = entry_blk()->add_imm(ImmValue(ImmType::IMM_I1));
    };

    undefined_initializer();
}

auto SSA_pass::entry_blk() -> Ir::Block * { return cur_func.front().get(); }

auto SSA_pass::blk_def(Ir::Block *block, vrtl_reg *blk_def_val) -> Ir::pUse {

    Ir::pUse blk_use =
        std::make_shared<Ir::Use>(block->label().get(), blk_def_val);
    blk_def_val->users.push_back(blk_use);
    return blk_use;
}

auto SSA_pass::undef_val(vrtl_reg *variable) -> Ir::Const * {
    auto pointee_ty = to_pointed_type(variable->ty);
    if (pointee_ty->type_type() == TYPE_BASIC_TYPE) {
        switch (to_basic_type(pointee_ty)->ty) {
        case IMM_I32:
            return dynamic_cast<Ir::Const *>(undefined_values.i32.get());
        case IMM_F32:
            return dynamic_cast<Ir::Const *>(undefined_values.f32.get());
        case IMM_I1:
            return dynamic_cast<Ir::Const *>(undefined_values.i1.get());
        default:
            break;
            ;
        }
    }
    my_assert(false, "undef val");
    return nullptr;
}

auto SSA_pass::erase_blk_def(Ir::pUse &blk_def_use) -> void {
    auto *blk_def_val = blk_def_use->usee;
    for (auto it = blk_def_val->users.begin(); blk_def_val->users.end() != it;
         it++) {
        if (*it == blk_def_use) {
            blk_def_val->users.erase(it);
            break;
        }
    }
}

// the defintion of $variable in $block is $blk_def_val
auto SSA_pass::def_val(vrtl_reg *variable, Ir::Block *block,
                       vrtl_reg *blk_def_val) -> void {
    my_assert(blk_def_val, "non-null def val");
    my_assert(dynamic_cast<Ir::Instr *>(blk_def_val) ||
                  dynamic_cast<Ir::Const *>(blk_def_val),
              "defintion type must be Instr or Const");
    if (blk_def_val->type() == Ir::VAL_INSTR) {
        if (static_cast<Ir::Instr *>(blk_def_val)->block() == nullptr)
            function_args.insert(blk_def_val);
    }
    if (current_def[variable].count(block) > 0) {
        erase_blk_def(current_def[variable][block]);
    }
    current_def[variable][block] = blk_def(block, blk_def_val);
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
        return current_def[variable][block]->usee;
    }
    if (block == entry_blk()) {
        // undef value
        return undef_val(variable);
    }
    auto ret = use_val_recursive(variable, block);
    my_assert(ret, "non-null");
    return ret;
}

SSA_pass::vrtl_reg *SSA_pass::use_val_recursive(vrtl_reg *variable,
                                                Ir::Block *block) {
    vrtl_reg *val = nullptr;
    if (sealedBlocks.find(block) == sealedBlocks.end()) {
        // Incomplete CFG.
        auto phi = Ir::make_phi_instr(variable->ty);
        val = phi.get();
        block->push_after_label(phi);
        // block->body.insert(std::next(block()->begin()), phi);
        // my_assert(block->body.at(1) == phi, "head insertion");
        incompletePhis[block].emplace_back(variable, phi);
    } else if (block->in_blocks().size() == 1) {
        // Optimize the common case of one predecessor: No phi needed
        val = use_val(variable, *block->in_blocks().begin());
    } else {
        my_assert(!block->in_blocks().empty(),
                  "every block in such function must have predecessor");
        // Break potential cycles with operandless phi
        auto phi = Ir::make_phi_instr(variable->ty);
        block->push_after_label(phi);
        // block->insert(std::next(block()->begin()), phi);
        // my_assert(block->body.at(1) == phi, "head insertion");
        def_val(variable, block, phi.get());
        val = addPhiOperands(variable, phi.get(), block);
    }
    def_val(variable, block, val);
    return val;
}

SSA_pass::vrtl_reg *SSA_pass::addPhiOperands(vrtl_reg *variable, Ir::Instr *phi,
                                             Ir::Block *phi_blk) {
    my_assert(phi->block() == phi_blk, "comes from the same block");
    auto *phi_ins = dynamic_cast<Ir::PhiInstr *>(phi);
    for (auto *pred : phi_blk->in_blocks()) {
        auto *pred_val = use_val(variable, pred);
        // phi->add_incoming(pred, val);
        phi_ins->add_incoming(pred->label().get(), pred_val);
    }
    return tryRemoveTrivialPhi(phi_ins);
}

auto SSA_pass::tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg * {

    vrtl_reg *same = nullptr;

    auto is_trivial = [&same, &phi]() -> bool {
        Set<Ir::Val *> trivial_phi_ops;
        // my_assert(phi->operands.size() == phi->labels.size(), "valid phi");
        for (size_t i = 0; i < phi->operand_size() / 2; ++i) {
            trivial_phi_ops.insert(phi->phi_val(i));
        }
        my_assert(!trivial_phi_ops.empty(), "non-empty operands");

        if (trivial_phi_ops.size() >= 3) {
            return false;
        }

        if (trivial_phi_ops.size() == 1) {
            same = *trivial_phi_ops.begin();
            // my_assert(same != phi, "none of operands can be phi itself");
            if (same == phi) {
                my_assert(phi->users.empty(), "trivial phi can not be used");
            }
            return true;
        }

        if (trivial_phi_ops.count(phi) == 0) {
            return false;
        }

        trivial_phi_ops.erase(phi);
        same = *trivial_phi_ops.begin();
        return true;
    };

    if (!is_trivial()) {
        return phi;
    }
    my_assert(same, "nullptr to some val in phi incoming tuples");
    Vector<Ir::PhiInstr *> phiUsers;

    auto phi_all_users = fmap<Ir::pUse, Ir::User *>(
        [&phi](auto arg_use) {
            my_assert(arg_use->usee == phi,
                      "the source of such use-def edge must be phi instr");
            return arg_use->user;
        },
        phi->users);
    for (Ir::User *user : phi_all_users) {
        if (user != phi && is_phi(user)) {
            my_assert(dynamic_cast<Ir::PhiInstr *>(user), "must be phi instr");
            phiUsers.push_back(static_cast<Ir::PhiInstr *>(user));
        }
    }

    // reset val
    phi->replace_self(same);

    auto remove_trivial = [](Ir::PhiInstr *&phi) {
        // remove op
        phi->block()->erase(phi);
    };
    remove_trivial(phi);
    for (auto *user : phiUsers) {
        if (user == same)
            same = tryRemoveTrivialPhi(user);
    }
    return same;
}

void SSA_pass::sealBlock(Ir::Block *block) {
    if (incompletePhis.find(block) != incompletePhis.end()) {
        for (auto [variable, phi] : incompletePhis[block]) {
            addPhiOperands(variable, phi.get(), block);
        }
        incompletePhis.erase(block);
    }
    sealedBlocks.insert(block);
}

inline auto SSA_pass::unreachable_blks() -> Set<Ir::Block *> {
    return dom_ctx.unreachable_blocks;
}

void SSA_pass::reconstruct() {
    if (!unreachable_blks().empty()) {
        std::cerr << "Unreachable blocks detected\n";
        for (auto unr_blk : dom_ctx.unreachable_blocks) {
            std::cerr << "Unreachable block: " << unr_blk->label()->name()
                      << "\n";
        }
    }

    auto const promotable_filter = [](Ir::Val *arg_alloca_instr) -> bool {
        auto pointee_ty = to_pointed_type(arg_alloca_instr->ty);
        return !is_array(pointee_ty) && !is_struct(pointee_ty) &&
               !(arg_alloca_instr->type() == Ir::VAL_GLOBAL);
    };

    // static pointer or run time initialized pointer
    auto const pointer_verifier =
        [this, &promotable_filter](Ir::Val *arg_non_alloca) -> bool {
        return !promotable_filter(arg_non_alloca) ||
               (is_pointer(arg_non_alloca->ty) &&
                dynamic_cast<Ir::ItemInstr *>(arg_non_alloca)) ||
               (function_args.count(arg_non_alloca) > 0);
    };

    Set<vrtl_reg *> alloca_vars;
    auto build_def = [this, &alloca_vars, &promotable_filter]() -> void {
        for (const auto &ent_instr : *entry_blk()) {
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
    std::transform(cur_func.begin(), cur_func.end(),
                   std::inserter(pred, pred.end()), [](const auto &bb) {
                       return std::make_pair(bb.get(), bb->in_blocks().size());
                   });
    auto trySeal = [this, &pred](Ir::Block *block) -> void {
        if (sealedBlocks.count(block) == 0 && pred[block] == 0) {
            sealBlock(block);
        };
    };

    auto mem_destination_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operand(0);
    };

    auto mem_sourve_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operand(1);
    };

    Vector<Ir::Block *> order;
    order.insert(order.begin(), dom_ctx.order().begin(), dom_ctx.order().end());

    for (auto *cur_block : order) {
        auto it = cur_block->begin();
        my_assert((*it)->instr_type() == Ir::INSTR_LABEL, "label begin"), it++;
        for (; it != cur_block->end();
             /* next perform increment*/) {
            auto succ_instr = std::next(it);
            auto cur_instr = *it;
            if (cur_instr->instr_type() == Ir::INSTR_STORE) {
                auto *store = dynamic_cast<Ir::StoreInstr *>(cur_instr.get());
                const auto &to = mem_destination_val(store);
                if (alloca_vars.count(to->usee) > 0) {
                    auto val = mem_sourve_val(store);
                    def_val(to->usee, cur_block, val->usee);
                    cur_block->erase(it);
                } else {
                    my_assert(pointer_verifier(to->usee),
                              "store to non-alloca variable");
                }

            } else if (cur_instr->instr_type() == Ir::INSTR_LOAD) {
                auto *load = dynamic_cast<Ir::LoadInstr *>(cur_instr.get());
                auto src_ptr = load->operand(0);
                if (alloca_vars.count(src_ptr->usee) > 0) {
                    load->replace_self(use_val(src_ptr->usee, cur_block));
                    my_assert(load->users.empty(), "removable load instr");
                    cur_block->erase(it);
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

    my_assert(sealedBlocks.size() == cur_func.size(), "all blocks are sealed");

    for (auto ent_instr_it = ++entry_blk()->begin();
         ent_instr_it != entry_blk()->end();) {
        if (auto *ent_instr =
                dynamic_cast<Ir::AllocInstr *>(ent_instr_it->get());
            ent_instr != nullptr) {
            if (promotable_filter(ent_instr)) {
                // my_assert(ent_instr->users.empty(), "single value is
                // removable");
                if (ent_instr->users.empty()) {
                    ent_instr_it = entry_blk()->erase(ent_instr_it);
                    continue;
                }
            }
        }
        ent_instr_it++;
    }

    for (auto [_, def_map] : current_def) {
        for (auto [_, blk_def_use] : def_map) {
            erase_blk_def(blk_def_use);
        }
    }
}

void SSA_pass::pass_transform() {
    my_assert(promotion_type == ssa_type::RECONSTRUCTION, "RECONS");
    reconstruct();
}

} // namespace Optimize