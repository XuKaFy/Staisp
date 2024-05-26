#include "trans_SSA.h"
#include "alys_dom.h"
#include "def.h"
#include "imm.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"
#include "type.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <unordered_map>
/*
This algorithem is based on the paper "Simple and Efficient Construction of
Static Single Assignment Form" by Braun, M., Buchwald, S., Hack, S., Leißa, R.,
Mallon, C., & Zwinkau, A. (2013).
https:doi.org/10.1007/978-3-642-37051-9_6
*/

namespace Optimize {

[[nodiscard]]
Ir::pInstr make_identity_instr(Ir::Val *&arg_val) {
    if (arg_val->type() != Ir::VAL_INSTR) {
        my_assert(arg_val->type() == Ir::VAL_CONST, "constant value type");
        auto const_val =
            Ir::make_constant(ImmValue(to_basic_type(arg_val->ty)->ty));
        auto arg_const =
            Ir::make_constant(dynamic_cast<Ir::Const *>(arg_val)->v);
        if (is_integer(arg_val->ty) || is_signed_type(arg_val->ty)) {
            return Ir::make_binary_instr(Ir::INSTR_ADD, const_val, arg_const);
        } else if (is_float(arg_val->ty)) {
            return Ir::make_binary_instr(Ir::INSTR_FADD, const_val, arg_const);
        }
        my_assert(false, "unreachable code path");
    }
    auto arg_instr = dynamic_cast<Ir::Instr *>(arg_val);
    my_assert(arg_instr, "must be instr");
    Ir::pVal val = nullptr;
    for (const auto &cur : arg_instr->block->body) {
        if (cur.get() == arg_instr) {
            val = cur;
            break;
        }
    }
    Ir::pVal id_val;
    Ir::pInstr ret;
    my_assert(val, "existenence of such val");
    my_assert(to_basic_type(val->ty), "must be value type");
    switch (to_basic_type(val->ty)->ty) {
    case IMM_I1:
    case IMM_I8:
        id_val = Ir::make_constant(ImmValue((int8_t)0));
    case IMM_I16:
        id_val = Ir::make_constant(ImmValue((int16_t)0));
    case IMM_I32:
        id_val = Ir::make_constant(ImmValue((int32_t)0));
    case IMM_I64:
        id_val = Ir::make_constant(ImmValue((int64_t)0));

        return Ir::make_binary_instr(Ir::INSTR_ADD, id_val, val);
        break;

    case IMM_F32:
        id_val = Ir::make_constant(ImmValue(0.0F));
    case IMM_F64:
        id_val = Ir::make_constant(ImmValue(0.0));

        return Ir::make_binary_instr(Ir::INSTR_FADD, id_val, val);
        break;

    case IMM_U1:
    case IMM_U8:
        id_val = Ir::make_constant(ImmValue((uint8_t)0));
    case IMM_U16:
        id_val = Ir::make_constant(ImmValue((uint16_t)0));
    case IMM_U32:
        id_val = Ir::make_constant(ImmValue((uint32_t)0));
    case IMM_U64:
        id_val = Ir::make_constant(ImmValue((uint64_t)0));

        return Ir::make_binary_instr(Ir::INSTR_ADD, id_val, val);
        break;
    }
}

SSA_pass::SSA_pass(Ir::BlockedProgram &cur_func, const ssa_type &pro_type)
    : promotion_type(pro_type), cur_func(cur_func) {
    dom_ctx.build_dom(cur_func);
#ifdef dom_bug
    dom_ctx.print_dom();
#endif
    cur_func.normal_opt();
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
        auto cur_instr = static_cast<Ir::Instr *>(user);
        return my_assert(cur_instr, "must be Instr *"),
               cur_instr->instr_type() == Ir::INSTR_PHI;
    }
    return false;
}

SSA_pass::vrtl_reg *SSA_pass::use_val(vrtl_reg *variable, Ir::Block *block) {
    if (current_def.at(variable).count(block)) {
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
        incompletePhis[block].emplace_back<Pair<vrtl_reg *, Ir::Instr *>>(
            {variable, phi.get()});
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
    auto phi_ins = dynamic_cast<Ir::PhiInstr *>(phi);
    for (auto pred : phi_blk->in_blocks()) {
        auto val = use_val(variable, pred);
        // phi->add_incoming(pred, val);
        phi_ins->add_incoming(pred, val);
    }
    return tryRemoveTrivialPhi(phi_ins);
}

auto SSA_pass::tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg * {
    vrtl_reg *same = nullptr;

    for (auto it : phi->operands) {
        auto val = it->usee;
        my_assert(it->user == phi, "semantics of phi USE");
        if (val == same || val == phi)
            continue;
        if (same)
            return phi;
        same = val;
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

    for (auto user : phiUsers) {
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
    for (auto bb : cur_func.blocks) {
        if (!dom_ctx.dom_map.count(bb.get())) {
            unreachable.insert(bb.get());
        }
    }
    return unreachable;
}

void SSA_pass::reconstruct() {
    my_assert(unreachable_blks().empty(), "no unreachable blocks");

    Set<vrtl_reg *> alloca_vars;
    auto build_def = [this, &alloca_vars]() -> void {
        for (auto ent_instr : entry_blk()->body) {
            if (ent_instr->instr_type() == Ir::INSTR_ALLOCA) {
                auto alloca = static_cast<Ir::AllocInstr *>(ent_instr.get());
                if (is_pointer(alloca->ty)) {
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
        if (!sealedBlocks.count(block) && pred[block] == 0) {
            sealBlock(block);
        };
    };

    auto mem_destination_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operands.at(0);
    };

    auto mem_sourve_val = [](Ir::StoreInstr *arg_instr) -> Ir::pUse {
        return arg_instr->operands.at(1);
    };

    // either elementptr or global or array
    auto type_filter = [](Ir::Val *arg_val) -> bool {
        return (is_pointer(arg_val->ty) &&
                static_cast<Ir::ItemInstr *>(arg_val)) ||
               is_array(arg_val->ty) || is_struct(arg_val->ty) ||
               arg_val->type() == Ir::VAL_GLOBAL;
    };

    Vector<Ir::Block *> order;
    order.insert(order.begin(), dom_ctx.order().begin(), dom_ctx.order().end());

    for (auto cur_block : order) {
        for (auto it = cur_block->body.begin(); it != cur_block->body.end();
             /* next perform increment*/) {
            auto succ_instr = std::next(it);
            auto cur_instr = *it;
            if (cur_instr->instr_type() == Ir::INSTR_STORE) {
                auto store = dynamic_cast<Ir::StoreInstr *>(cur_instr.get());
                auto to = mem_destination_val(store);
                if (alloca_vars.count(to->usee)) {
                    auto val = mem_sourve_val(store);
                    def_val(to->usee, cur_block, val->usee);
                    cur_block->body.erase(it);
                } else {
                    my_assert(type_filter(to->usee),
                              "store to non-alloca variable");
                }

            } else if (cur_instr->instr_type() == Ir::INSTR_LOAD) {
                auto load = static_cast<Ir::LoadInstr *>(cur_instr.get());
                auto src_ptr = load->operands.at(0);
                if (alloca_vars.count(src_ptr->usee)) {
                    load->replace_self(use_val(src_ptr->usee, cur_block));
                    my_assert(load->users.empty(), "removable load instr");
                    cur_block->body.erase(it);
                } else {
                    my_assert(type_filter(src_ptr->usee),
                              "load from non-alloca variable");
                }
            }
            it = succ_instr;
        }
        trySeal(cur_block);
        for (auto succ_bb : cur_block->out_blocks()) {
            pred[succ_bb]--;
            trySeal(succ_bb);
        }
    }

    my_assert(sealedBlocks.size() == cur_func.blocks.size(),
              "all blocks are sealed");
}

void SSA_pass::pass_transform() {
    if (promotion_type == ssa_type::RECONSTRUCTION)
        reconstruct();
    else
        my_assert(false, "unreachable code path");
}

} // namespace Optimize