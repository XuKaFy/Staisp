
#include "trans_loop.h"
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include "ir_call_instr.h"
#include "ir_cmp_instr.h"
#include "ir_constant.h"
#include "ir_control_instr.h"
#include "ir_func_defined.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "ir_phi_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"
#include "trans_SSA.h"
#include "type.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <set>

namespace Optimize {

inline void alter_cmp(Ir::CmpType &cmp_type) {
    switch (cmp_type) {

    case Ir::CMP_SGE:
        cmp_type = Ir::CMP_SLE;
        break;
    case Ir::CMP_SLE:
        cmp_type = Ir::CMP_SGE;
        break;
    case Ir::CMP_SGT:
        cmp_type = Ir::CMP_SLT;
        break;
    case Ir::CMP_SLT:
        cmp_type = Ir::CMP_SGT;
        break;

    case Ir::CMP_OGE:
        cmp_type = Ir::CMP_OLE;
        break;
    case Ir::CMP_OLE:
        cmp_type = Ir::CMP_OGE;
        break;
    case Ir::CMP_OGT:
        cmp_type = Ir::CMP_OLT;
        break;
    case Ir::CMP_OLT:
        cmp_type = Ir::CMP_OGT;
        break;

    case Ir::CMP_UGE:
        cmp_type = Ir::CMP_ULE;
        break;
    case Ir::CMP_ULE:
        cmp_type = Ir::CMP_UGE;
        break;
    case Ir::CMP_ULT:
        cmp_type = Ir::CMP_UGT;
        break;
    case Ir::CMP_UGT:
        cmp_type = Ir::CMP_ULT;
        break;

    case Ir::CMP_UNE:
    case Ir::CMP_OEQ:
    case Ir::CMP_NE:
    case Ir::CMP_EQ:
        break;
    }
}

auto inline true_label(Ir::BrCondInstr *br_cond) {
    return dynamic_cast<Ir::LabelInstr *>(br_cond->operand(1)->usee);
}

auto inline false_label(Ir::BrCondInstr *br_cond) {
    return dynamic_cast<Ir::LabelInstr *>(br_cond->operand(2)->usee);
}

auto branch_label_replace(Ir::BrCondInstr *br_cond, Ir::LabelInstr *old_label,
                          Ir::LabelInstr *new_label) {
    if (true_label(br_cond) == old_label) {
        br_cond->change_operand(1, new_label);
        return;
    } else if (false_label(br_cond) == old_label) {
        br_cond->change_operand(2, new_label);
        return;
    }
    my_assert(false, "replace non-existed label");
}

void Canonicalizer_pass::ap() {
    auto dom_set = Alys::build_dom_set(dom_ctx);

    auto negate_cmp = [](Ir::CmpType &cmp_type) {
        switch (cmp_type) {
        case Ir::CMP_SGE:
            cmp_type = Ir::CMP_SLT;
            break;
        case Ir::CMP_SLT:
            cmp_type = Ir::CMP_SGE;
            break;
        case Ir::CMP_SLE:
            cmp_type = Ir::CMP_SGT;
            break;
        case Ir::CMP_SGT:
            cmp_type = Ir::CMP_SLE;
            break;

        case Ir::CMP_OGE:
            cmp_type = Ir::CMP_OLT;
            break;
        case Ir::CMP_OLE:
            cmp_type = Ir::CMP_OGT;
            break;
        case Ir::CMP_OGT:
            cmp_type = Ir::CMP_OLE;
            break;
        case Ir::CMP_OLT:
            cmp_type = Ir::CMP_OGE;
            break;

        case Ir::CMP_UGE:
            cmp_type = Ir::CMP_ULT;
            break;
        case Ir::CMP_ULE:
            cmp_type = Ir::CMP_UGT;
            break;
        case Ir::CMP_ULT:
            cmp_type = Ir::CMP_UGE;
            break;
        case Ir::CMP_UGT:
            cmp_type = Ir::CMP_ULE;
            break;

        case Ir::CMP_UNE:
            cmp_type = Ir::CMP_OEQ;
            break;
        case Ir::CMP_OEQ:
            cmp_type = Ir::CMP_UNE;
            break;
        case Ir::CMP_NE:
            cmp_type = Ir::CMP_EQ;
            break;
        case Ir::CMP_EQ:
            cmp_type = Ir::CMP_NE;
            break;
        }
    };

    for (auto cur_bb : cur_func) {
        auto back_instr = cur_bb->back();
        if (back_instr->instr_type() != Ir::INSTR_BR_COND) {
            continue;
        }

        auto loop_cond = back_instr->operand(0)->usee;
        auto loop_comparator = dynamic_cast<Ir::CmpInstr *>(loop_cond);
        if (!loop_comparator) {
            continue;
        }

        auto const_val = [](Ir::Val *&val) -> bool {
            return val->type() == Ir::VAL_CONST;
        };

        if (const_val(loop_comparator->operand(0)->usee) &&
            !const_val(loop_comparator->operand(1)->usee)) {
            auto op1 = loop_comparator->operand(0)->usee;
            auto op2 = loop_comparator->operand(1)->usee;
            // TODO: rewrite op swap
            loop_comparator->release_operand(0);
            loop_comparator->release_operand(0);
            loop_comparator->add_operand(op2);
            loop_comparator->add_operand(op1);
            alter_cmp(loop_comparator->cmp_type);
        }

        auto l_label =
            dynamic_cast<Ir::LabelInstr *>(back_instr->operand(1)->usee);
        auto r_label =
            dynamic_cast<Ir::LabelInstr *>(back_instr->operand(2)->usee);

        if (l_label != r_label && loop_comparator->users().size() == 1 &&
            Alys::is_dom(r_label->block(), cur_bb.get(), dom_set) &&
            !Alys::is_dom(l_label->block(), cur_bb.get(), dom_set)) {
            back_instr->release_operand(1);
            back_instr->release_operand(1);
            back_instr->add_operand(r_label);
            back_instr->add_operand(l_label);
            negate_cmp(loop_comparator->cmp_type);
        }
    }
}

bool IndVarPruning_pass::is_Mono(Ir::Val *val, Ir::Block *loop_hdr, bool dir,
                                 int depth) {

    auto val_as_instr = dynamic_cast<Ir::Instr *>(val);

    std::function<bool(Ir::Val *, Ir::Val *&)> add_tautof =
        [](Ir::Val *op_val, Ir::Val *&left_exactor) -> bool {
        left_exactor = op_val;
        return true;
    };

    std::function<bool(Ir::Val *, Ir::Const *&)> add_signed_constf =
        [](Ir::Val *op_val, Ir::Const *&right_exactor) -> bool {
        if (is_signed_type(op_val->ty) && op_val->type() == Ir::VAL_CONST) {
            right_exactor = dynamic_cast<Ir::Const *>(op_val);
            return true;
        }
        return false;
    };

    std::function<bool(Ir::Val *, Ir::Val *&)> add_idf =
        [](Ir::Val *op_val, Ir::Val *&arg_val) -> bool {
        return arg_val == op_val;
    };
    if (val_as_instr->instr_type() == Ir::INSTR_PHI) {
        auto phi_instr = dynamic_cast<Ir::PhiInstr *>(val_as_instr);
        constexpr intmax_t maxIncrement = 1;
        for (auto [blk_label, phi_usee] : *phi_instr) {
            if (is_invariant(phi_usee, loop_hdr))
                continue;

            Ir::Const *step;
            if (binary_extractor<Ir::Val *, Ir::Const *, Ir::INSTR_ADD, true>(
                    phi_usee, val, step, add_idf, add_signed_constf))
                return false;

            auto step_as_int = step->v.imm_value().val.ival;
            if (dir) {
                if (step_as_int <= 0)
                    return false;
                if (step_as_int <= maxIncrement)
                    continue;
            } else {
                if (step_as_int >= 0)
                    return false;
                if (step_as_int >= -maxIncrement)
                    continue;
            }
            return false;
        }
        return true;
    }
    if (depth < 0) {
        return false;
    }
    Ir::Val *acc;
    Ir::Const *inc;

    if (binary_extractor<Ir::Val *, Ir::Const *, Ir::INSTR_ADD, true>(
            val, acc, inc, add_tautof, add_signed_constf)) {
        return is_Mono(val, loop_hdr, dir, depth - 1);
    }
    return false;
}

bool IndVarPruning_pass::is_pure(Ir::Block *arg_blk) {
    auto is_pure_instr = [](const Ir::pInstr &arg_instr) -> bool {
        if (arg_instr->users().empty())
            return false;

        my_assert(arg_instr->instr_type() != Ir::INSTR_STORE, "non-store");
        if (arg_instr->instr_type() == Ir::INSTR_CALL) {
            auto call_instr = dynamic_cast<Ir::CallInstr *>(arg_instr.get());
            my_assert(call_instr->func_ty->type_type() != TYPE_VOID_TYPE,
                      "non void ret");
            auto callee = call_instr->operand(0)->usee;
            auto callee_func = dynamic_cast<Ir::FuncDefined *>(callee);
            return callee_func && callee_func->is_pure();
        }
        return true;
    };
    for (const auto &instr : *arg_blk) {
        if (instr->is_terminator())
            continue;
        if (!is_pure_instr(instr) || instr->instr_type() == Ir::INSTR_LOAD)
            return false;
    }
    return true;
}

bool IndVarPruning_pass::is_closure_loop(Ir::Block *block, Ir::Block *exit) {
    for (auto cur_instr : *block) {
        for (auto &&instr_Use : cur_instr->users()) {
            auto cur_user = instr_Use->user;
            if (auto usr_instr = dynamic_cast<Ir::Instr *>(cur_user);
                usr_instr) {
                if (Alys::is_dom(exit, usr_instr->block(), dom_set))
                    return true;

                if (usr_instr->instr_type() == Ir::INSTR_PHI) {
                    auto phi = dynamic_cast<Ir::PhiInstr *>(usr_instr);
                    for (auto [pred_blk, val] : *phi) {
                        if (val == cur_instr.get()) {
                            if (Alys::is_dom(exit, pred_blk->block(), dom_set))
                                return true;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void IndVarPruning_pass::ap() {
    std::function<bool(Ir::Val *, Ir::Val *&)> icmp_tautof =
        [](Ir::Val *op_val, Ir::Val *&left_exactor) -> bool {
        left_exactor = op_val;
        return true;
    };

    for (auto cur_blk : cur_func) {
        auto back_instr = cur_blk->back();
        if (back_instr->instr_type() != Ir::INSTR_BR_COND)
            continue;

        auto cur_cnd_br = dynamic_cast<Ir::BrCondInstr *>(back_instr.get());

        Ir::CmpType cmp_op;
        Ir::Val *lhs, *rhs, *ind_var;
        auto icmp_usee = cur_cnd_br->operand(0)->usee;
        if (!cmp_extractor<Ir::Val *, Ir::Val *, true>(
                icmp_usee, cmp_op, lhs, rhs, icmp_tautof, icmp_tautof))
            continue;

        auto true_tar = true_label(cur_cnd_br);
        if (true_tar == false_label(cur_cnd_br))
            continue;

        auto is_removable_cmp = [&true_tar, &cur_blk,
                                 this](Ir::Val *v1, Ir::Val *v2) -> bool {
            Ir::Block *hdr_blk;
            if (v1->type() != Ir::VAL_CONST) {
                auto v1_as_instr = dynamic_cast<Ir::Instr *>(v1);
                hdr_blk = v1_as_instr->block();
            } else {
                hdr_blk = nullptr;
            }

            if (!hdr_blk || hdr_blk != true_tar->block())
                return false;
            if (hdr_blk == cur_blk.get())
                return false;
            if (!Alys::is_dom(cur_blk.get(), hdr_blk, dom_set))
                return false;
            if (!is_invariant(v2, hdr_blk))
                return false;
            return true;
        };

        if (is_removable_cmp(lhs, rhs)) {
            ind_var = lhs;
        } else if (is_removable_cmp(rhs, lhs)) {
            ind_var = rhs;
            alter_cmp(cmp_op);
        } else {
            continue;
        }

        auto ind_as_instr = dynamic_cast<Ir::Instr *>(ind_var);
        const auto hdr_blk = ind_as_instr->block();

        constexpr int depth = 1;

        if (cmp_op == Ir::CMP_SGT || cmp_op == Ir::CMP_SGE) {
            if (!is_Mono(ind_var, hdr_blk, true, depth))
                continue;
        } else if (cmp_op == Ir::CMP_SLT || cmp_op == Ir::CMP_SLE) {
            if (!is_Mono(ind_var, hdr_blk, false, depth))
                continue;
        } else {
            continue;
        }

        if (!(is_pure(hdr_blk) && is_pure(hdr_blk)))
            continue;

        const auto hdr_back = hdr_blk->back();

        if (hdr_back->instr_type() != Ir::INSTR_BR_COND)
            continue;

        auto hdr_cnd_br = dynamic_cast<Ir::BrCondInstr *>(hdr_back.get());

        if (true_label(hdr_cnd_br)->block() != cur_blk.get() &&
            false_label(hdr_cnd_br)->block() != cur_blk.get())
            continue;

        auto exit = true_label(hdr_cnd_br)->block() == cur_blk.get()
                        ? false_label(hdr_cnd_br)->block()
                        : true_label(hdr_cnd_br)->block();

        if (exit == cur_blk.get() || exit == hdr_blk ||
            exit == false_label(cur_cnd_br)->block())
            continue;

        if (is_closure_loop(hdr_blk, exit) ||
            is_closure_loop(cur_blk.get(), exit))
            continue;

        branch_label_replace(cur_cnd_br, hdr_blk->label().get(),
                             exit->label().get());

        auto phi_rewrite = [&cur_blk](Ir::Block *old_lb, Ir::Block *new_lb) {
            for (auto instr : *cur_blk) {
                if (instr->instr_type() == Ir::INSTR_PHI) {
                    auto phi_instr = dynamic_cast<Ir::PhiInstr *>(instr.get());
                    for (auto [blk_label, phi_usee] : *phi_instr) {
                        if (blk_label == old_lb->label().get()) {
                            phi_instr->change_phi_label(0,
                                                        new_lb->label().get());
                        }
                    }
                }
            }
        };

        phi_rewrite(cur_blk.get(), hdr_blk);
    }
}

LoopGEPMotion_pass::LoopGEPMotion_pass(Ir::BlockedProgram &arg_func,
                                       Alys::DomTree &arg_dom)
    : dom_ctx(arg_dom), cur_func(arg_func), loop_ctx(cur_func, dom_ctx) {
    dom_set = Alys::build_dom_set(dom_ctx);
    for (auto [loop_hdr, _] : loop_ctx.loops) {
        auto prehdr_blk = Ir::make_block();

        prehdr_blk->push_back(Ir::make_label_instr());

        auto in_blks = loop_hdr->in_blocks();
        for (auto in_it = in_blks.begin(); in_it != in_blks.end();) {
            if (Alys::is_dom(*in_it, loop_hdr, dom_set)) {
                in_it = in_blks.erase(in_it);
            } else {
                in_it++;
            }
        }

        for (auto &hdr_instr : *loop_hdr) {
            if (hdr_instr->instr_type() == Ir::INSTR_PHI) {
                auto cur_phi = dynamic_cast<Ir::PhiInstr *>(hdr_instr.get());
                auto pre_clone_instr = Ir::make_phi_instr(cur_phi->ty);
                prehdr_blk->push_after_label(pre_clone_instr);
                auto pre_clone_phi =
                    dynamic_cast<Ir::PhiInstr *>(pre_clone_instr.get());
                new_phi_instrs.push_back(pre_clone_phi);
                // transfer phi incoming
                for (auto [lb, val] : *cur_phi) {
                    if (in_blks.count(lb->block()) > 0) {
                        pre_clone_phi->add_incoming(lb, val);
                    }
                }
                // remove old labels
                cur_phi->add_incoming(prehdr_blk->label().get(), pre_clone_phi);
                for (auto in_blk : in_blks) {
                    cur_phi->remove(in_blk->label().get());
                }
            }
        }

        prehdr_blk->push_back(Ir::make_br_instr(loop_hdr->label().get()));
        for (auto in_blk : in_blks) {
            auto bk_instr = in_blk->back();
            if (bk_instr->instr_type() == Ir::INSTR_BR_COND) {
                auto br_cond = dynamic_cast<Ir::BrCondInstr *>(bk_instr.get());
                if (true_label(br_cond) == loop_hdr->label().get()) {
                    branch_label_replace(br_cond, loop_hdr->label().get(),
                                         prehdr_blk->label().get());
                } else if (false_label(br_cond) == loop_hdr->label().get()) {
                    branch_label_replace(br_cond, loop_hdr->label().get(),
                                         prehdr_blk->label().get());
                }
            } else {
                my_assert(bk_instr->instr_type() == Ir::INSTR_BR, "no ret");
                bk_instr->change_operand(0, prehdr_blk->label().get());
            }
        }
        for (auto blk_it = cur_func.begin(); blk_it != cur_func.end();
             ++blk_it) {
            if ((*blk_it).get() == loop_hdr) {
                cur_func.insert(blk_it, prehdr_blk);
                break;
            }
        }
    }
    dom_ctx = Alys::DomTree();
    dom_ctx.build_dom(cur_func);
    dom_set = Alys::build_dom_set(dom_ctx);
}

void LoopGEPMotion_pass::process_cur_blk(Ir::Block *arg_blk,
                                         Ir::Block *loop_hdr) {

    auto pred_blk_move = [&loop_hdr, this](Ir::Instr *arg_instr) {
        auto pred_blk = dom_ctx.dom_map[loop_hdr]->idom->basic_block;
        instr_move(arg_instr, pred_blk);
    };
#ifdef USING_MINI_GEP
    auto is_gep_invariant = [this,
                             &loop_hdr](Ir::MiniGepInstr *cur_item) -> bool {
#else
    auto is_gep_invariant = [this, &loop_hdr](Ir::ItemInstr *cur_item) -> bool {
#endif
        auto base_val = cur_item->operand(0)->usee;
        if (!((base_val->type() == Ir::VAL_GLOBAL && is_array(base_val->ty)) ||
              is_func_parameter(base_val, cur_func) ||
              is_invariant(base_val, loop_hdr, dom_set)))
            return false;
        for (size_t i = 1; i < cur_item->operand_size(); ++i) {
            auto cur_offset = cur_item->operand(i)->usee;
            if (is_func_parameter(cur_offset, cur_func))
                continue;
            if (!LoopGEPMotion_pass::is_invariant(cur_offset, loop_hdr,
                                                  dom_set))
                return false;
        }
        return true;
    };

    Vector<Ir::pInstr> arg_blk_instrs;
    for (auto instr : *arg_blk) {
        arg_blk_instrs.push_back(instr);
    }
    for (auto cur_instr : arg_blk_instrs) {
#ifdef USING_MINI_GEP
        if (cur_instr->instr_type() == Ir::INSTR_MINI_GEP) {
            auto cur_item = dynamic_cast<Ir::MiniGepInstr *>(cur_instr.get());
#else
        if (cur_instr->instr_type() == Ir::INSTR_ITEM) {
            auto cur_item = dynamic_cast<Ir::ItemInstr *>(cur_instr.get());
#endif
            if (!is_gep_invariant(cur_item))
                continue;
            auto cur_base = cur_item->operand(0)->usee;

            if (aliases.count(cur_base) == 0) {
#ifdef USING_MINI_GEP
                auto cur_val = aliases[cur_base] =
                    std::set<Ir::MiniGepInstr *, decltype(item_cmp)>(item_cmp);
#else
                auto cur_val = aliases[cur_base] =
                    std::set<Ir::ItemInstr *, decltype(item_cmp)>(item_cmp);

#endif
                cur_val.insert(cur_item);
                hoistable_gep.insert(cur_item);
                pred_blk_move(cur_item);
            } else {
                if (auto [real_gep_it, result] =
                        aliases[cur_base].insert(cur_item);
                    !result) {
                    auto real_gep = *real_gep_it;
                    if (Alys::is_dom(real_gep->block(), cur_item->block(),
                                     dom_set) &&
                        cur_item->block() != real_gep->block()) {
                        hoistable_gep.erase(*real_gep_it);
                        hoistable_gep.insert(cur_item);
                        aliases[cur_base].erase(real_gep_it);
                        aliases[cur_base].insert(cur_item);
                        real_gep->replace_self(cur_item);
                        real_gep->block()->erase(real_gep);
                    } else if (Alys::is_dom(cur_item->block(),
                                            real_gep->block(), dom_set)) {
                        cur_item->replace_self(*real_gep_it);
                        cur_item->block()->erase(cur_item);
                    }
                } else {
                    hoistable_gep.insert(cur_item);
                    pred_blk_move(cur_item);
                }
            }
        } else {
            switch (cur_instr->instr_type()) {

            case Ir::INSTR_CAST:
            case Ir::INSTR_CMP:
            case Ir::INSTR_BINARY:
            case Ir::INSTR_UNARY:
            case Ir::INSTR_ITEM:
            case Ir::INSTR_MINI_GEP:

                arithmetic_ap(
                    cur_instr.get(),
                    [&loop_hdr, this, &pred_blk_move](Ir::Instr *cur_instr) {
                        auto flag = true;
                        for (size_t i = 0; i < cur_instr->operand_size(); i++) {
                            auto cur_op = cur_instr->operand(i)->usee;
                            if (!is_invariant(cur_op, loop_hdr, dom_set)) {
                                flag = false;
                                break;
                            }
                        }
                        if (flag) {
                            pred_blk_move(cur_instr);
                        }
                    });

            // item and ctrl instrs
            case Ir::INSTR_LOAD:

            case Ir::INSTR_SYM:
            case Ir::INSTR_LABEL:
            case Ir::INSTR_BR:
            case Ir::INSTR_BR_COND:
            case Ir::INSTR_FUNC:
            case Ir::INSTR_CALL:
            case Ir::INSTR_RET:
            case Ir::INSTR_ALLOCA:
            case Ir::INSTR_STORE:
            case Ir::INSTR_PHI:
            case Ir::INSTR_UNREACHABLE:
                break;
            }
        }
    }
}

void LoopGEPMotion_pass::ap() {

    auto is_in_cur_loop =
        [](Ir::Block *arg_blk,
           const Set<Ir::Block *> &loop_blks_predicate) -> bool {
        return loop_blks_predicate.find(arg_blk) != loop_blks_predicate.end();
    };
    for (auto &&[cur_hdr, cur_body] : loop_ctx.loops) {
        auto cur_blk = cur_hdr;
        auto loop_predicate = cur_body->loop_blocks;
        Vector<Ir::Block *> blk_stack;
        blk_stack.push_back(cur_blk);
        while (!blk_stack.empty()) {
            cur_blk = blk_stack.back();
            blk_stack.pop_back();
            if (!is_in_cur_loop(cur_blk, loop_predicate))
                continue;
            process_cur_blk(cur_blk, cur_hdr);
            for (auto dom_succs : dom_ctx.dom_map[cur_blk]->out_block) {
                blk_stack.push_back(dom_succs->basic_block);
            }
        }

        auto pred_blk = dom_ctx.dom_map[cur_hdr]->idom->basic_block;
#ifdef USING_MINI_GEP
        auto gep_tobe_moved = Vector<Ir::MiniGepInstr *>{hoistable_gep.begin(),
                                                         hoistable_gep.end()};
        std::sort(gep_tobe_moved.begin(), gep_tobe_moved.end(),
                  [](Ir::MiniGepInstr *lhs, Ir::MiniGepInstr *rhs) -> bool {
                      return lhs->instr_print() < rhs->instr_print();
                  });
#else
        auto gep_tobe_moved =
            Vector<Ir::ItemInstr *>{hoistable_gep.begin(), hoistable_gep.end()};
        std::sort(gep_tobe_moved.begin(), gep_tobe_moved.end(),
                  [](Ir::ItemInstr *lhs, Ir::ItemInstr *rhs) -> bool {
                      return lhs->instr_print() < rhs->instr_print();
                  });
#endif
        for (auto item : gep_tobe_moved) {
            // printf("%s \t use cnt: %zu\n", item->instr_print().c_str(),
            //        item->users.size());
            // instr_move(item, pred_blk);
            if (item->users().size() == 1) {
                auto cur_user = item->users().front()->user;
                if (auto store = dynamic_cast<Ir::StoreInstr *>((cur_user));
                    store) {
                    auto store_val_src = store->operand(1)->usee;
                    if (LoopGEPMotion_pass::is_invariant(store_val_src, cur_hdr,
                                                         dom_set))
                        instr_move(store, pred_blk);
                }
            }
        }
        hoistable_gep.clear();
    }

    clean_up();
}

void instr_move(Ir::Instr *cur_instr, Ir::Block *new_blk) {
    bool succ = false;
    auto old_blk = cur_instr->block();
    for (const auto &instr : *old_blk) {
        if (instr.get() == cur_instr) {
            new_blk->push_behind_end(instr);
            old_blk->erase(instr.get());
            succ = true;
            break;
        }
    }
    my_assert(succ, "move successfully");
}

void LoopGEPMotion_pass::clean_up() {
    for (auto &&phi : new_phi_instrs) {
        SSA_pass::tryRemoveTrivialPhi(phi);
    }
}
} // namespace Optimize