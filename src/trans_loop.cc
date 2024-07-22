
#include "trans_loop.h"
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_constant.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "ir_phi_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cstdint>
#include <functional>

namespace Optimize {

void Canonicalizer_pass::ap() {
    auto dom_set = Alys::build_dom_set(dom_ctx);
    auto changed = false;
    auto alter_cmp = [](Ir::CmpType &cmp_type) {
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
    };

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
            my_assert(back_instr->instr_type() == Ir::INSTR_BR ||
                          back_instr->instr_type() == Ir::INSTR_RET,
                      "br instr");
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
            changed = true;
        }

        auto l_label =
            dynamic_cast<Ir::LabelInstr *>(back_instr->operand(1)->usee);
        auto r_label =
            dynamic_cast<Ir::LabelInstr *>(back_instr->operand(2)->usee);

        if (l_label != r_label && loop_comparator->users.size() == 1 &&
            Alys::is_dom(r_label->block(), cur_bb.get(), dom_set) &&
            !Alys::is_dom(l_label->block(), cur_bb.get(), dom_set)) {
            back_instr->release_operand(1);
            back_instr->release_operand(1);
            back_instr->add_operand(r_label);
            back_instr->add_operand(l_label);
            negate_cmp(loop_comparator->cmp_type);
            changed = true;
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

    std::function<bool(Ir::Val *, const Ir::Val *&)> add_idf =
        [](Ir::Val *op_val, const Ir::Val *&arg_val) -> bool {
        return arg_val == op_val;
    };
    if (val_as_instr->instr_type() == Ir::INSTR_PHI) {
        auto phi_instr = dynamic_cast<Ir::PhiInstr *>(val_as_instr);
        constexpr intmax_t maxIncrement = 1;
        for (auto [blk_label, phi_usee] : *phi_instr) {
            if (is_invariant(phi_usee, loop_hdr))
                continue;

            Ir::Const *step;
            if (addtion_verifier(phi_usee, val, step, add_idf,
                                 add_signed_constf))
                return false;
            if (dir) {
                if (step->v.imm_value() <= 0)
                    return false;
                if (step <= maxIncrement)
                    continue;
            } else {
                if (step >= 0)
                    return false;
                if (step >= -maxIncrement)
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

    if (addtion_verifier(val, acc, inc, add_tautof, add_signed_constf)) {
        return is_Mono(val, loop_hdr, dir, depth - 1);
    }
    return false;
}

template <typename Tl, typename Tr>
bool addtion_verifier(Ir::Val *val, Tl &lhs, Tr &rhs,
                      const std::function<bool(Ir::Val *, Tl &)> &lf,
                      const std::function<bool(Ir::Val *, Tr &)> &rf) {
    auto val_as_instr = dynamic_cast<Ir::Instr *>(val);

    if (val_as_instr->instr_type() != Ir::INSTR_BINARY)
        return false;
    auto bin_instr = dynamic_cast<Ir::BinInstr *>(val_as_instr);
    if (bin_instr->binType != Ir::INSTR_ADD)
        return false;
    auto fst_op = bin_instr->operand(0)->usee;
    auto snd_op = bin_instr->operand(1)->usee;

    if (lf(fst_op, lhs) && rf(snd_op, rhs)) {
        return true;
    }
    if (lf(snd_op, lhs) && rf(fst_op, rhs)) {
        return true;
    }

    return false;
}

} // namespace Optimize