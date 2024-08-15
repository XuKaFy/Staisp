#include "opt.h"

#include <reg2mem.h>

#include "alys_dom.h"
#include "def.h"
#include "ir_func_defined.h"
#include "opt_DFA.h"
#include "opt_DSE.h"
#include "opt_GVN.h"
#include "opt_const_propagate.h"
#include "opt_interprocedural.h"
#include "trans_SSA.h"
#include "trans_loop.h"

namespace Optimize {

int func_pass_dse(const Ir::pFuncDefined &func) {
    int cnt = from_bottom_analysis<OptDSE::BlockValue,
                                   OptDSE::TransferFunction>(func->p);
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("DSE") == 0, "DSE Failed");
    return cnt;
}

int func_pass_const_propagate(const Ir::pFuncDefined &func) {
    int cnt = from_top_analysis<OptConstPropagate::BlockValue,
                                OptConstPropagate::TransferFunction>(func->p);
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("CP") == 0, "CP Failed");
    return cnt;
}

void func_pass_ssa(const Ir::pFuncDefined &func) {
    SSA_pass(func->p).reconstruct();
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("SSA") == 0, "SSA Failed");
}

void func_pass_canonicalizer(const Ir::pFuncDefined &func, Alys::DomTree &dom_ctx)
{
    Optimize::Canonicalizer_pass(func->p, dom_ctx).ap();
    my_assert(func->p.check_empty_use("Canonicalizer") == 0,
              "Canonicalizer Failed");
    func->p.re_generate();
}

void func_pass_loop_gep_motion(const Ir::pFuncDefined &func, Alys::DomTree &dom_ctx)
{
    Optimize::LoopGEPMotion_pass(func->p, dom_ctx).ap();
    my_assert(func->p.check_empty_use("LoopGEPMotion") == 0,
              "LoopGEPMotion Failed");
    func->p.re_generate();
}

void func_pass_pointer_iteration(const Ir::pFuncDefined &func, Alys::DomTree &dom_ctx) {
    pointer_iteration(func->p, dom_ctx);
    func->p.plain_opt_all();
    func->p.re_generate();
}

void func_pass_gvn(const Ir::pFuncDefined &func, Alys::DomTree &dom_ctx) {
    OptGVN::GVN_pass(func->p, dom_ctx).ap();
    func->p.re_generate();
}

Alys::DomTree func_pass_get_dom_ctx(const Ir::pFuncDefined &func) {
    Alys::DomTree dom_ctx;
    dom_ctx.build_dom(func->p);
    func->p.re_generate();
    return dom_ctx;
}

void pass_inline(const Ir::pModule &mod) {
    mod->remove_unused_function();
    while (inline_all_function(mod))
        ;
    mod->remove_unused_function();
}

void pass_dfa(const Ir::pModule &mod) {
    for (auto &&func : mod->funsDefined) {
        int cnt = 0;
        const int MAX_OPT_COUNT = 8;
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = func_pass_dse(func);
            opt_cnt += func_pass_const_propagate(func);
        }
    }
}

void optimize(const Ir::pModule &mod) {
    pass_inline(mod);
    for (auto &&func : mod->funsDefined) {
        func_pass_const_propagate(func);
    }
    global2local(mod);
    for (auto &&func : mod->funsDefined) {
        func_pass_ssa(func);
    }
    pass_dfa(mod);
    for (auto &&func : mod->funsDefined) {
        Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
        func_pass_canonicalizer(func, dom_ctx);
        func_pass_loop_gep_motion(func, dom_ctx);
        func_pass_pointer_iteration(func, dom_ctx);
        func_pass_gvn(func, dom_ctx);
    }
}

} // namespace Optimize
