
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "opt.h"
#include "type.h"
#include <algorithm>
#include <functional>

/*
This algorithem is based on the paper "Simple and Efficient Construction of
Static Single Assignment Form" by Braun, M., Buchwald, S., Hack, S., Leißa, R.,
Mallon, C., & Zwinkau, A. (2013).
https:doi.org/10.1007/978-3-642-37051-9_6
*/

namespace Optimize {
class SSA_pass {

    using vrtl_reg = Ir::Val;
    Map<vrtl_reg *, Map<Ir::Block *, vrtl_reg *>> current_def;
    Set<Ir::Block *> sealedBlocks;
    Ir::BlockedProgram &cur_func;
    Alys::DomTree dom_ctx;
    Map<Ir::Block *, Vector<Pair<vrtl_reg *, Ir::Instr *>>> incompletePhis;

    SSA_pass(Ir::BlockedProgram &cur_func) : cur_func(cur_func) {
        dom_ctx.build_dom(cur_func);
    }

    auto entry_blk() -> Ir::Block * { return cur_func.blocks.front().get(); }

    auto def_val(vrtl_reg *variable, Ir::Block *block, vrtl_reg *val) -> void {
        current_def[variable][block] = val;
    }

    template <typename a, typename b>
    auto fmap(std::function<b(a)> f, Vector<a> v1) -> Vector<b> {
        Vector<b> v2;
        for (auto vs : v1) {
            v2.push_back(f(vs));
        }
        return v2;
    };

    auto is_phi(Ir::User *user) -> bool {
        if (user->type() == Ir::VAL_INSTR) {
            auto t = static_cast<Ir::Instr *>(user);
            my_assert(t, "t must be Instr *");
            return t->instr_type() == Ir::INSTR_PHI;
        }
        return false;
    }

    vrtl_reg *use_val(vrtl_reg *variable, Ir::Block *block) {
        if (current_def.at(variable).count(block)) {
            return current_def[variable][block];
        }
        my_assert(
            block != entry_blk(),
            "The definition of every variable must be in the entry block.");
        return use_val_recursive(variable, block);
    }

    vrtl_reg *use_val_recursive(vrtl_reg *variable, Ir::Block *block) {
        vrtl_reg *val = nullptr;
        if (sealedBlocks.find(block) == sealedBlocks.end()) {
            // Incomplete CFG.
            auto phi = Ir::make_phi_instr(variable->ty);
            my_assert(phi->instr_type() == Ir::INSTR_PHI,
                      "phi instr type is not INSTR_PHI");
            val = phi.get();
            block->body.insert(block->body.cend(), phi);
            my_assert(block->body.front() == phi, "head insertion");
            incompletePhis[block].emplace_back<Pair<vrtl_reg *, Ir::Instr *>>(
                {variable, phi.get()});
        } else if (block->in_block.size() == 1) {
            // Optimize the common case of one predecessor: No phi needed
            val = use_val(variable, *block->in_block.begin());
        } else {
            my_assert(!block->in_block.empty(),
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

    vrtl_reg *addPhiOperands(vrtl_reg *variable, Ir::Instr *phi,
                             Ir::Block *phi_blk) {
        auto phi_ins = static_cast<Ir::PhiInstr *>(phi);
        for (auto pred : phi_blk->in_block) {
            auto val = use_val(variable, pred);
            // phi->add_incoming(pred, val);
            phi_ins->add_incoming(pred, val);
        }
        return tryRemoveTrivialPhi(phi_ins);
    }

    auto tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg * {
        vrtl_reg *same = nullptr;
        for (auto [_, use_val] : phi->incoming_tuples) {
            auto val = use_val->usee;
            if (val == same || val == phi)
                continue;
            if (same)
                return phi;
            same = val;
        }

        my_assert(same, "nullptr to some val in phi incoming tuples");
        Vector<Ir::PhiInstr *> phiUsers;

        for (Ir::User *user : fmap<Ir::pUse, Ir::User *>(
                 [](auto pU) { return pU->user; }, phi->users)) {
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

    void sealBlock(Ir::Block *block) {
        if (incompletePhis.find(block) != incompletePhis.end()) {
            for (auto [variable, phi] : incompletePhis[block]) {
                addPhiOperands(variable, phi, block);
            }
            incompletePhis.erase(block);
        }
        sealedBlocks.insert(block);
    }

    auto unreachable_blks() -> Set<Ir::Block *> {
        Set<Ir::Block *> unreachable;
        for (auto bb : cur_func.blocks) {
            if (!dom_ctx.dom_map.count(bb.get()))
                unreachable.insert(bb.get());
        }
        return unreachable;
    }

    void transform_func() {
        my_assert(unreachable_blks().empty(), "no unreachable blocks");
        for (auto cur_block : cur_func.blocks) {
            for (auto cur_instr : cur_block->body) {
            }
        }
    }
};
} // namespace Optimize