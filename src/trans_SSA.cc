
#include "trans_SSA.h"
#include "def.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_call_instr.h"
#include "type.h"
#include <algorithm>
#include <iterator>
/*
This algorithem is based on the paper "Simple and Efficient Construction of
Static Single Assignment Form" by Braun, M., Buchwald, S., Hack, S., Leißa, R.,
Mallon, C., & Zwinkau, A. (2013).
https:doi.org/10.1007/978-3-642-37051-9_6
*/

namespace Optimize {

SSA_pass::SSA_pass(Ir::BlockedProgram &cur_func) : cur_func(cur_func) {
    dom_ctx.build_dom(cur_func);
}

auto SSA_pass::entry_blk() -> Ir::Block * {
    return cur_func.blocks.at(0).get();
}

auto SSA_pass::def_val(vrtl_reg *variable, Ir::Block *block,
                       vrtl_reg *definition_val) -> void {
    current_def[variable][block] = definition_val;
}

auto SSA_pass::is_phi(Ir::User *user) -> bool {
    if (user->type() == Ir::VAL_INSTR) {
        auto *t = static_cast<Ir::Instr *>(user);
        my_assert(t, "t must be Instr *");
        return t->instr_type() == Ir::INSTR_PHI;
    }
    return false;
}

SSA_pass::vrtl_reg *SSA_pass::use_val(vrtl_reg *variable, Ir::Block *block) {
    if (current_def.at(variable).count(block) != 0U) {
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
        my_assert(is_phi(phi.get()), "phi instr type is not INSTR_PHI");
        val = phi.get();
        block->body.insert(block->body.cend(), phi);
        my_assert(block->body.front() == phi, "head insertion");
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
        block->body.insert(block->body.cend(), phi);
        my_assert(block->body.front() == phi, "head insertion");
        def_val(variable, block, phi.get());
        val = addPhiOperands(variable, phi.get(), block);
    }
    def_val(variable, block, val);
    return val;
}

SSA_pass::vrtl_reg *SSA_pass::addPhiOperands(vrtl_reg *variable, Ir::Instr *phi,
                                             Ir::Block *phi_blk) {
    auto *phi_ins = static_cast<Ir::PhiInstr *>(phi);
    for (auto *pred : phi_blk->in_blocks()) {
        auto *val = use_val(variable, pred);
        // phi->add_incoming(pred, val);
        phi_ins->add_incoming(pred, val);
    }
    return tryRemoveTrivialPhi(phi_ins);
}

auto SSA_pass::tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg * {
    vrtl_reg *same = nullptr;
    for (auto [_, use_val] : phi->incoming_tuples) {
        auto *val = use_val->usee;
        if (val == same || val == phi) {
            continue;
}
        if (same != nullptr) {
            return phi;
}
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
    for (const auto& bb : cur_func.blocks) {
        if (dom_ctx.dom_map.count(bb.get()) == 0U) {
            unreachable.insert(bb.get());
        }
    }
    return unreachable;
}

void SSA_pass::transform_func() {
    my_assert(unreachable_blks().empty(), "no unreachable blocks");

    auto use_map = [this](Ir::Instr *instr, Ir::Block *block) -> void {
        switch (instr->instr_type()) {
        case Ir::INSTR_CALL:
        case Ir::INSTR_CMP:
        case Ir::INSTR_RET:
        case Ir::INSTR_UNARY:
        case Ir::INSTR_BINARY:
            for (const auto& arg : instr->operands) {
                arg->usee->replace_self(use_val(arg->usee, block));
            }
            break;

        case Ir::INSTR_STORE: {
            auto *store = static_cast<Ir::StoreInstr *>(instr);
            auto *to = store->operands.at(0)->usee;
            auto *val = store->operands.at(1)->usee;
            // val must be a pointer type
            my_assert(to_pointer_type(val->ty), "pointer type");
            to->replace_self(use_val(to, block));
            break;
        }

        case Ir::INSTR_ITEM:
        case Ir::INSTR_LOAD: {
            auto *use = static_cast<Ir::Instr *>(instr)->operand(0)->usee;
            use->replace_self(use_val(use, block));
            break;
        }

        case Ir::INSTR_PHI:
        case Ir::INSTR_CAST:
        case Ir::INSTR_ALLOCA:
            break;

        // instruction without implementation / unrelated
        case Ir::INSTR_SYM:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_BR:
        case Ir::INSTR_BR_COND:
        case Ir::INSTR_FUNC:
        case Ir::INSTR_UNREACHABLE:
            break;
        };
    };
    auto def_map = [this](Ir::Instr *instr, Ir::Block *block) -> void {
        switch (instr->instr_type()) {
        case Ir::INSTR_CALL: {
            auto *call = static_cast<Ir::CallInstr *>(instr);
            if (call->ty->type_type() != TYPE_VOID_TYPE) {
                def_val(instr, block, instr);
}
            break;
        }
        case Ir::INSTR_CMP:
        case Ir::INSTR_UNARY:
        case Ir::INSTR_ITEM:
        case Ir::INSTR_BINARY:
        case Ir::INSTR_LOAD:
        case Ir::INSTR_ALLOCA:
            def_val(instr, block, instr);
            break;

        case Ir::INSTR_STORE: {
            auto *store = static_cast<Ir::StoreInstr *>(instr);
            auto *destination_ptr = store->operands.at(0)->usee;
            auto *val = store->operands.at(1)->usee;
            my_assert(to_pointer_type(destination_ptr->ty), "pointer type");
            def_val(destination_ptr, block, val);
            break;
        }
        case Ir::INSTR_CAST:
        case Ir::INSTR_RET:
        case Ir::INSTR_PHI:
            break;
        case Ir::INSTR_SYM:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_BR:
        case Ir::INSTR_BR_COND:
        case Ir::INSTR_FUNC:
        case Ir::INSTR_UNREACHABLE:
            break;
        };
    };

    Map<Ir::Block *, size_t> pred = {};
    std::transform(cur_func.blocks.begin(), cur_func.blocks.end(),
                   std::inserter(pred, pred.end()), [](const auto &bb) {
                       return std::make_pair(bb.get(), bb->in_blocks().size());
                   });
    auto trySeal = [this, &pred](Ir::Block *block) -> void {
        if ((sealedBlocks.count(block) == 0U) && pred[block] == 0) {
            sealBlock(block);
        };
    };

    for (const auto& cur_block : cur_func.blocks) {
        for (const auto& cur_instr : cur_block->body) {
            use_map(cur_instr.get(), cur_block.get());
            def_map(cur_instr.get(), cur_block.get());
        }
        trySeal(cur_block.get());
        for (auto *succ_bb : cur_block->out_blocks()) {
            pred[succ_bb]--;
            trySeal(succ_bb);
        }
    }

    my_assert(sealedBlocks.size() == cur_func.blocks.size(),
              "all blocks are sealed");
}
} // namespace Optimize