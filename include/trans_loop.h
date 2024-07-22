
#pragma once

#include "alys_dom.h"
#include "alys_loop.h"
#include "def.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
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
    IndVarPruning_pass(Ir::BlockedProgram &arg_func, Alys::DomTree &arg_dom,
                       Map<Ir::Block *, Set<Ir::Block *>> arg_dom_set = {})
        : cur_func(arg_func), dom_ctx(arg_dom) {
        if (arg_dom_set.empty())
            dom_set = Alys::build_dom_set(dom_ctx);
        dom_set = arg_dom_set;
    }

    void ap();

    bool is_invariant(Ir::Val *val, Ir::Block *loop_hdr) {
        if (val->type() == Ir::VAL_CONST)
            return true;

        my_assert(val->type() == Ir::VAL_INSTR, "instruction as value");
        auto val_as_instr = dynamic_cast<Ir::Instr *>(val);
        return val_as_instr->block() != loop_hdr;
    }

    bool is_Mono(Ir::Val *val, Ir::Block *loop_hdr, bool dir, int depth);
};

template <typename Tl, typename Tr>
bool addtion_verifier(Ir::Val *val, Tl &lhs, Tr &rhs,
                      const std::function<bool(Ir::Val *, Tl &)> &lf,
                      const std::function<bool(Ir::Val *, Tr &)> &rf);
} // namespace Optimize