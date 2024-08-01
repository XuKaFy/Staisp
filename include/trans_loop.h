
#pragma once

#include "alys_dom.h"
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "ir_val.h"
namespace Optimize {
class Canonicalizer_pass {
    Ir::BlockedProgram &cur_func;
    Alys::DomTree &dom_ctx;

public:
    Canonicalizer_pass(Ir::BlockedProgram &arg_func, Alys::DomTree &arg_dom)
        : cur_func(arg_func), dom_ctx(arg_dom) {}
    void ap();
    void rewrite_induction();
};

class IndVarPruning_pass {
    Ir::BlockedProgram &cur_func;
    Alys::DomTree &dom_ctx;
    Map<Ir::Block *, Set<Ir::Block *>> dom_set;

public:
    IndVarPruning_pass(Ir::BlockedProgram &arg_func, Alys::DomTree &arg_dom)
        : cur_func(arg_func), dom_ctx(arg_dom) {
        dom_set = Alys::build_dom_set(dom_ctx);
    }

    void ap();

    bool is_invariant(Ir::Val *val, Ir::Block *loop_hdr) {
        if (val->type() == Ir::VAL_CONST)
            return true;

        my_assert(val->type() == Ir::VAL_INSTR, "instruction as value");
        auto val_as_instr = dynamic_cast<Ir::Instr *>(val);
        return val_as_instr->block() != loop_hdr &&
               Alys::is_dom(loop_hdr, val_as_instr->block(), dom_set);
    }

    bool is_Mono(Ir::Val *val, Ir::Block *loop_hdr, bool dir, int depth);
    bool is_pure(Ir::Block *arg_blk);
    bool is_closure_loop(Ir::Block *block, Ir::Block *exit);
};

template <typename Tl, typename Tr, Ir::BinInstrType exactee, bool is_abelian>
bool binary_extractor(Ir::Val *val, Tl &lhs, Tr &rhs,
                      const std::function<bool(Ir::Val *, Tl &)> &lf,
                      const std::function<bool(Ir::Val *, Tr &)> &rf) {
    auto val_as_instr = dynamic_cast<Ir::Instr *>(val);

    if (val_as_instr->instr_type() != Ir::INSTR_BINARY)
        return false;
    auto bin_instr = dynamic_cast<Ir::BinInstr *>(val_as_instr);
    if (bin_instr->binType != exactee)
        return false;
    auto fst_op = bin_instr->operand(0)->usee;
    auto snd_op = bin_instr->operand(1)->usee;

    if (lf(fst_op, lhs) && rf(snd_op, rhs)) {
        return true;
    }
    if (is_abelian)
        if (lf(snd_op, lhs) && rf(fst_op, rhs)) {
            return true;
        }

    return false;
}

template <typename Tl, typename Tr, bool is_icmp>
[[nodiscard]]
bool cmp_extractor(Ir::Val *val, Ir::CmpType &instr_cmp_op, Tl &lhs, Tr &rhs,
                   const std::function<bool(Ir::Val *, Tl &)> &lf,
                   const std::function<bool(Ir::Val *, Tr &)> &rf) {
    auto val_as_instr = dynamic_cast<Ir::Instr *>(val);

    if (val_as_instr->instr_type() != Ir::INSTR_CMP)
        return false;

    auto cmp_instr = dynamic_cast<Ir::CmpInstr *>(val_as_instr);
    instr_cmp_op = cmp_instr->cmp_type;
    auto fst_op = cmp_instr->operand(0)->usee;
    if (is_icmp != is_integer(fst_op->ty))
        return false;
    auto snd_op = cmp_instr->operand(1)->usee;

    if (lf(fst_op, lhs) && rf(snd_op, rhs)) {
        return true;
    }

    return false;
}
} // namespace Optimize