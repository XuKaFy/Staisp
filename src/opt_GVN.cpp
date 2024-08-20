#include "opt_GVN.h"
#include "def.h"
#include "imm.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_phi_instr.h"
#include "ir_val.h"
#include "trans_loop.h"
#include "type.h"
#include <cstddef>
#include <cstdio>
#include <deque>
#include <functional>
#include <set>

#define printf(...)

namespace OptGVN {

GVN_pass::GVN_pass(Ir::BlockedProgram &arg_func, Alys::DomTree &dom_ctx)
    : dom_ctx(dom_ctx), cur_func(arg_func) {
    dom_set = Alys::build_dom_set(dom_ctx);
    auto rpo_builder = [this]() -> auto {
        auto rpo = std::deque<Ir::Block *>{};
        size_t rpo_num = cur_func.size() + 3;
        Set<Ir::Block *> visited;
        auto dfs_build = [&visited, this, &rpo_num](
                             auto dfs_build, std::deque<Ir::Block *> &rpo,
                             Ir::Block *cur_blk) -> void {
            if (visited.count(cur_blk) > 0)
                return;
            visited.insert(cur_blk);
            for (auto out : cur_blk->out_blocks()) {
                dfs_build(dfs_build, rpo, out);
            }
            rpo_number[cur_blk] = rpo_num--;
            rpo.push_front(cur_blk);
        };

        dfs_build(dfs_build, rpo, cur_func.begin()->get());
        return rpo;
    };

    for (auto entry_instr : *cur_func.front())
        touched.insert(entry_instr.get());

    rpo = rpo_builder();
}

Exp GVN_pass::perform_symbolic_evaluation(Ir::Instr *arg_instr,
                                          Ir::Block *arg_blk) {
    Exp instr_exp;
    instr_exp.instr = arg_instr;
    if (arg_instr->instr_type() == Ir::INSTR_MINI_GEP) {
        // infer val todo
        instr_exp.args = collect_usees(arg_instr);
    }
    instr_exp.fold(exp_const);
    return instr_exp;
}

void GVN_pass::perform_congruence_finding(Ir::Instr *arg_instr, Exp *exp) {

    if (auto [it, result] = exp_pool.insert(*exp); !result) {
        auto real_instr = it->instr;
        if (!hoist_fold(real_instr, arg_instr)) {
            auto cloned_exp = *exp;
            cloned_exp.instr = arg_instr;
            exp_pool.erase(it);
            exp_pool.insert(cloned_exp);
            real_instr->replace_self(arg_instr);
            real_instr->block()->erase(real_instr);
        }
    } else if (arg_instr->instr_type() == Ir::INSTR_MINI_GEP ||
               arg_instr->instr_type() == Ir::INSTR_ITEM) {
        shared_gep.push_back(arg_instr);
    }
}

void Exp::fold(Map<Exp *, Ir::Const *> &exp_pool) {
    Vector<ImmValue> imm_args;
    for (auto arg : args) {
        if (arg->type() == Ir::VAL_CONST) {
            imm_args.push_back(dynamic_cast<Ir::Const *>(arg)->v.imm_value());
            continue;
        } else if (arg->type() == Ir::VAL_GLOBAL && is_pointer(arg->ty)) {
            continue;
        }
        return;
    }
    is_folded = true;
    if (auto cal_instr = dynamic_cast<Ir::CalculatableInstr *>(instr);
        cal_instr && imm_args.size() == instr->operand_size()) {
        auto imm_result = cal_instr->calculate(imm_args);
        auto folded_const = instr->block()->add_imm(imm_result).get();
        exp_pool[this] = static_cast<Ir::Const *>(folded_const);
    }
    // is_folded = true;
}

void GVN_pass::ap() {

    Vector<Ir::Block *> process_ed;
    while (!touched.empty()) {
        for (auto cur_blk : rpo) {

            Exp sym_result{};
            auto symbolic_evaluation = [&cur_blk, this,
                                        &sym_result](Ir::Instr *arg_instr) {
                sym_result = perform_symbolic_evaluation(arg_instr, cur_blk);
                perform_congruence_finding(arg_instr, &sym_result);
            };
            printf("%s\n", cur_blk->label()->name().c_str());
            Vector<Ir::pInstr> instr_set{cur_blk->begin(), cur_blk->end()};
            for (auto cur_instr : instr_set) {
                handle_cur_instr(cur_instr, cur_blk, symbolic_evaluation);
            }
        }
    }
    sink_gep();
}

void GVN_pass::handle_cur_instr(
    Ir::pInstr cur_instr, Ir::Block *cur_blk,
    std::function<void(Ir::Instr *)> symbolic_evaluation) {
    if (touched.count(cur_instr.get())) {
        touched.erase(cur_instr.get());
        switch (cur_instr->instr_type()) {

        case Ir::INSTR_ITEM:
        case Ir::INSTR_MINI_GEP: {
            symbolic_evaluation(cur_instr.get());
            break;
        }

        case Ir::INSTR_CAST:
        case Ir::INSTR_CMP:
        case Ir::INSTR_UNARY:
        case Ir::INSTR_BINARY: {
            Optimize::arithmetic_ap(cur_instr.get(), symbolic_evaluation);
            break;
        }

        case Ir::INSTR_BR_COND:
        case Ir::INSTR_BR: {
            process_out_blks(cur_blk);
            break;
        }

            // not need to handle
        case Ir::INSTR_PHI: {
            phi_predication(cur_instr.get());
        }
        case Ir::INSTR_CALL:
        case Ir::INSTR_ALLOCA:
        case Ir::INSTR_STORE:
        case Ir::INSTR_LOAD:
            break;
        case Ir::INSTR_SYM:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_FUNC:
        case Ir::INSTR_RET:
        case Ir::INSTR_UNREACHABLE:
            break;
        }
    }
}
void GVN_pass::process_out_blks(Ir::Block *arg_blk) {
    for (auto out_block : arg_blk->out_blocks()) {
        if (reachable.count(out_block) == 0) {
            reachable.insert(out_block);
            for (auto out_instr : *out_block)
                touched.insert(out_instr.get());

        } else {
            for (auto out_instr : *out_block) {
                if (out_instr->instr_type() == Ir::INSTR_PHI)
                    touched.insert(out_instr.get());
            }
            if (Alys::is_dom(out_block, arg_blk, dom_set)) {
                for (auto out_instr : *out_block)
                    touched.insert(out_instr.get());
            }
        }
    }
}

bool GVN_pass::is_block_definable(Ir::Block *arg_blk, Ir::Instr *arg_instr,
                                  DomPredicate dom_set,
                                  Ir::BlockedProgram &cur_func) {
    for (auto usee : collect_usees(arg_instr)) {
        switch (usee->type()) {
        case Ir::VAL_CONST:
        case Ir::VAL_GLOBAL: {
            continue;
        }
        case Ir::VAL_BLOCK:
        case Ir::VAL_FUNC:
            break;
        case Ir::VAL_INSTR: {
            if (Optimize::LoopGEPMotion_pass::is_func_parameter(usee, cur_func))
                continue;
            auto cur_usee_instr = static_cast<Ir::Instr *>(usee);
            if (!Alys::is_dom(arg_blk, cur_usee_instr->block(), dom_set))
                return false;
            continue;
        }
        }
    }
    return true;
};

Vector<Ir::Val *> GVN_pass::collect_usees(Ir::Instr *instr) {
    Vector<Ir::Val *> usees;
    for (size_t i = 0; i < instr->operand_size(); ++i) {
        usees.push_back(instr->operand(i)->usee);
    }
    return usees;
}

bool exp_eq::operator()(const Exp &a, const Exp &b) const {

    auto lhs_instr = a.instr;
    auto rhs_instr = b.instr;
    if (lhs_instr->instr_type() < rhs_instr->instr_type())
        return true;
    if (lhs_instr->operand_size() < rhs_instr->operand_size())
        return true;
    else if (lhs_instr->operand_size() > rhs_instr->operand_size())
        return false;
    if (lhs_instr->instr_type() == Ir::INSTR_BINARY) {
        auto bin_lhs = static_cast<Ir::BinInstr *>(lhs_instr);
        auto bin_rhs = static_cast<Ir::BinInstr *>(rhs_instr);
        if (bin_lhs->binType < bin_rhs->binType)
            return true;
        else if (bin_lhs->binType > bin_rhs->binType)
            return false;
        else {
            switch (bin_lhs->binType) {
            // abelian operation
            case Ir::INSTR_ADD:
            case Ir::INSTR_XOR:
            case Ir::INSTR_AND:
            case Ir::INSTR_OR:
            case Ir::INSTR_MUL:
            case Ir::INSTR_FADD:
            case Ir::INSTR_FMUL: {
                auto lhs_usees = GVN_pass::collect_usees(lhs_instr);
                auto rhs_usees = GVN_pass::collect_usees(rhs_instr);
                if (lhs_usees[0] == rhs_usees[0] &&
                    lhs_usees[1] == rhs_usees[1])
                    return false;
                else if (lhs_usees[0] == rhs_usees[1] &&
                         lhs_usees[1] == rhs_usees[0])
                    return false;
                else
                    return true;
                break;
            }

            case Ir::INSTR_SUB:
            case Ir::INSTR_FSUB:
            case Ir::INSTR_SDIV:
            case Ir::INSTR_SREM:
            case Ir::INSTR_UDIV:
            case Ir::INSTR_UREM:
            case Ir::INSTR_FDIV:
            case Ir::INSTR_FREM:
            case Ir::INSTR_ASHR:
            case Ir::INSTR_LSHR:
            case Ir::INSTR_SHL:
            case Ir::INSTR_SLT:
                break;
            }
        }
    }

    if (lhs_instr->instr_type() == Ir::INSTR_CMP) {
        auto cmp_lhs = static_cast<Ir::CmpInstr *>(lhs_instr);
        auto cmp_rhs = static_cast<Ir::CmpInstr *>(rhs_instr);
        if (cmp_lhs->cmp_type < cmp_rhs->cmp_type)
            return true;
        else if (cmp_lhs->cmp_type > cmp_rhs->cmp_type)
            return false;
    }

    if (lhs_instr->instr_type() == Ir::INSTR_MINI_GEP) {
        auto gep_lhs = static_cast<Ir::MiniGepInstr *>(lhs_instr);
        auto gep_rhs = static_cast<Ir::MiniGepInstr *>(rhs_instr);
        if (gep_lhs->in_this_dim < gep_rhs->in_this_dim)
            return true;
        else if (gep_lhs->in_this_dim > gep_rhs->in_this_dim)
            return false;
    }

    for (size_t i = 0; i < lhs_instr->operand_size(); ++i) {
        if (lhs_instr->operand(i)->usee < rhs_instr->operand(i)->usee)
            return true;
    }

    return false;
}

void GVN_pass::sink_gep() {
    auto dumpGepOperand = [](const String &str) -> String {
        return str.substr(str.find('g'));
    };
    Map<String, Ir::Instr *> gep_map;
    for (auto &&cur_gep : shared_gep) {
        auto cur_gep_str = dumpGepOperand(cur_gep->instr_print());
        printf("%s\n", cur_gep_str.c_str());
        if (auto [real_instr_it, result] =
                gep_map.insert({cur_gep_str, cur_gep});
            !result) {
            auto real_instr = real_instr_it->second;
            if (!hoist_fold(real_instr, cur_gep)) {
                real_instr->replace_self(cur_gep);
                real_instr->block()->erase(real_instr);
                gep_map.erase(real_instr_it);
                gep_map.insert({cur_gep_str, cur_gep});
            }
            continue;
        }
        printf("%s\n", cur_gep->instr_print().c_str());
    }
    for (auto &&[_, cur_gep] : gep_map) {
        auto cur_dom = cur_gep->block();
        for (auto &&cur_gep_user : cur_gep->users()) {
            auto user_ins = static_cast<Ir::Instr *>(cur_gep_user->usee);
            cur_dom = dom_ctx.LCA(user_ins->block(), cur_dom);
        }
        if (cur_dom != cur_gep->block())
            Optimize::instr_move(cur_gep, cur_dom);
    }
}

bool GVN_pass::hoist_fold(Ir::Instr *&real_instr, Ir::Instr *&arg_instr) {
    auto target = dom_ctx.LCA(real_instr->block(), arg_instr->block());
    my_assert(is_block_definable(target, real_instr, dom_set, cur_func) &&
                  is_block_definable(target, arg_instr, dom_set, cur_func),
              "use semantics");
    if (target == arg_instr->block()) {
        // arg sdom real
        if (real_instr->block() != arg_instr->block())
            return false;
        // arg is real
    }
    if (real_instr->block() != target)
        Optimize::instr_move(real_instr, target);
    arg_instr->replace_self(real_instr);
    arg_instr->block()->erase(arg_instr);
    return true;
}

void GVN_pass::phi_predication(Ir::Instr *arg_phi_instr) {
    auto arg_phi = static_cast<Ir::PhiInstr *>(arg_phi_instr);
    auto cur_blk = arg_phi->block();
    if (phi_in_blks.count(arg_phi->block()) == 0) {
        phi_in_blks[cur_blk] = Vector<Ir::PhiInstr *>{arg_phi};
        return;
    }

    auto &cur_phis = phi_in_blks[cur_blk];
    /*
    0 dominating values as initial
    1 dominated values as user
    2 nither 0 or 1
    */
    Vector<Ir::Val *> arg_vals[3];
    auto global_defined = [this](Ir::Val *val) -> bool {
        return val->type() == Ir::VAL_GLOBAL || val->type() == Ir::VAL_CONST ||
               Optimize::LoopGEPMotion_pass::is_func_parameter(val, cur_func);
    };
    auto val_filter =
        [this, // they share the same block as assumed
         cur_blk, global_defined](Ir::PhiInstr *arg_phi,
                                  Vector<Ir::Val *>(&val_of_phis)[3]) -> void {
        for (auto &&[lb, val] : *arg_phi) {
            // val >> arg_phi
            if (global_defined(val)) {
                val_of_phis[0].push_back(val);
                continue;
            } else if (val->type() == Ir::VAL_INSTR) {
                auto val_instr = static_cast<Ir::Instr *>(val);
                if (dom_ctx.is_dom(cur_blk, val_instr->block())) {
                    val_of_phis[0].push_back(val);
                } else if (dom_ctx.is_dom(val_instr->block(), cur_blk)) {
                    val_of_phis[1].push_back(val);
                } else {
                    val_of_phis[2].push_back(val);
                }
            }
        }
    };

    auto is_same_calculation = [](Ir::Val *cur, Ir::Val *arg,
                                  Ir::PhiInstr *curphi,
                                  Ir::PhiInstr *argphi) -> bool {
        auto cur_instr = dynamic_cast<Ir::Instr *>(cur);
        auto arg_instr = dynamic_cast<Ir::Instr *>(arg);
        if (cur_instr->instr_type() != arg_instr->instr_type())
            return false;
        if (cur_instr->operand_size() != arg_instr->operand_size())
            return false;
        if (cur_instr->instr_type() == Ir::INSTR_MINI_GEP) {
            auto cur_gep = dynamic_cast<Ir::MiniGepInstr *>(cur_instr);
            auto arg_gep = dynamic_cast<Ir::MiniGepInstr *>(arg_instr);
            if (cur_gep->in_this_dim != arg_gep->in_this_dim)
                return false;
            if (cur_gep->operand_size() != arg_gep->operand_size())
                return false;
            for (size_t i = 0; i < cur_gep->operand_size(); ++i) {
                auto cur_gep_usee = cur_gep->operand(i)->usee;
                auto arg_gep_usee = arg_gep->operand(i)->usee;
                if (cur_gep_usee->type() != cur_gep_usee->type()) {
                    return false;
                }
                if (cur_gep_usee->type() == Ir::VAL_CONST) {
                    auto cur_gep_const =
                        dynamic_cast<Ir::Const *>(cur_gep_usee);
                    auto arg_gep_const =
                        dynamic_cast<Ir::Const *>(arg_gep_usee);
                    if (cur_gep_const->v.imm_value() !=
                        arg_gep_const->v.imm_value())
                        return false;
                    continue;
                } else if (cur_gep_usee->type() == Ir::VAL_INSTR) {
                    if (!(cur_gep_usee == curphi && arg_gep_usee == argphi))
                        return false;
                    continue;
                }
            }
            return true;
        }
        return false;
    };

    val_filter(arg_phi, arg_vals);
    for (auto &&cur_phi : cur_phis) {
        if (cur_phi == arg_phi)
            continue;
        if (!is_same_type(cur_phi->ty, arg_phi->ty))
            continue;
        Vector<Ir::Val *> cur_vals[3];
        val_filter(cur_phi, cur_vals);
        if (!(cur_vals[0].size() == arg_vals[0].size() &&
              cur_vals[1].size() == arg_vals[1].size() &&
              cur_vals[2].size() == arg_vals[2].size()))
            continue;
        Set<Ir::Val *> vals_0_arg{arg_vals[0].begin(), arg_vals[0].end()};
        Set<Ir::Val *> vals_0_cur{cur_vals[0].begin(), cur_vals[0].end()};
        // same initializator
        Set<Pair<Ir::LabelInstr *, Ir::Val *>> arg, cur;
        for (size_t i = 0; i < arg_phi->phi_pairs(); ++i) {
            if (vals_0_arg.count(arg_phi->phi_val(i)) > 0) {
                arg.insert({arg_phi->phi_label(i), arg_phi->phi_val(i)});
            }
            if (vals_0_cur.count(cur_phi->phi_val(i)) > 0) {
                cur.insert({cur_phi->phi_label(i), cur_phi->phi_val(i)});
            }
        }
        if (arg != cur)
            continue;
        if (!(cur_vals[2].empty() && arg_vals[2].empty()))
            continue;
        if (cur_vals[1].size() != arg_vals[1].size())
            continue;
        if (cur_vals[1].size() >= 2)
            continue;
        if (is_same_calculation(cur_vals[1].at(0), arg_vals[1].at(0), cur_phi,
                                arg_phi)) {
            // printf("cur : %s\n arg : %s \n", cur_phi->instr_print().c_str(),
            //        arg_phi->instr_print().c_str());
            arg_phi->replace_self(cur_phi);
            cur_blk->erase(arg_phi);
            return;
        }
    }
    cur_phis.push_back(arg_phi);
}
} // namespace OptGVN