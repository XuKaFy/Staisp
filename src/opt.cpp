#include "opt.h"

#include <reg2mem.h>

#include "alys_dom.h"
#include "def.h"
#include "ir_func_defined.h"
#include "opt_DFA.h"
#include "opt_DSE.h"
#include "opt_GVN.h"
#include "opt_const_propagate.h"
#include "opt_ptr_iter.h"
#include "opt_interprocedural.h"
#include "opt_array_accumulate.h"
#include "trans_SSA.h"
#include "trans_loop.h"

namespace Optimize {

DFAAnalysisData<OptDSE::BlockValue> func_pass_dse(const Ir::pFuncDefined &func) {
    auto ans = from_bottom_analysis<OptDSE::BlockValue,
                                    OptDSE::TransferFunction>(func->p);
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("DSE") == 0, "DSE Failed");
    return ans;
}

DFAAnalysisData<OptConstPropagate::BlockValue> func_pass_const_propagate(const Ir::pFuncDefined &func) {
    auto ans = from_top_analysis<OptConstPropagate::BlockValue,
                                 OptConstPropagate::TransferFunction>(func->p);
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("CP") == 0, "CP Failed");
    return ans;
}

void func_pass_ssa(const Ir::pFuncDefined &func) {
    SSA_pass(func->p).reconstruct();
    func->p.plain_opt_all();
    my_assert(func->p.check_empty_use("SSA") == 0, "SSA Failed");
    func->p.re_generate();
}

Alys::DomTree func_pass_get_dom_ctx(const Ir::pFuncDefined &func) {
    Alys::DomTree dom_ctx;
    dom_ctx.build_dom(func->p);
    return dom_ctx;
}

void func_pass_canonicalizer(const Ir::pFuncDefined &func)
{
    Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
    Optimize::Canonicalizer_pass(func->p, dom_ctx).ap();
    my_assert(func->p.check_empty_use("Canonicalizer") == 0,
              "Canonicalizer Failed");
    func->p.re_generate();
}

void func_pass_loop_gep_motion(const Ir::pFuncDefined &func)
{
    Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
    Optimize::LoopGEPMotion_pass(func->p, dom_ctx).ap();
    my_assert(func->p.check_empty_use("LoopGEPMotion") == 0,
              "LoopGEPMotion Failed");
    func->p.re_generate();
}

void func_pass_pointer_iteration(const Ir::pFuncDefined &func) {
    Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
    pointer_iteration(func->p, dom_ctx);
    func->p.plain_opt_all();
    func->p.re_generate();
}


void func_pass_loop_unrolling(const Ir::pFuncDefined &func) {
    Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
    loop_unrolling(func->p, dom_ctx);
    func->p.re_generate();
}

void func_pass_gvn(const Ir::pFuncDefined &func) {
    Alys::DomTree dom_ctx = func_pass_get_dom_ctx(func);
    OptGVN::GVN_pass(func->p, dom_ctx).ap();
    func->p.re_generate();
}

void func_pass_array_accumulate(const Ir::pFuncDefined &func)
{
    // only get cp data
    auto ans = from_top_analysis<OptConstPropagate::BlockValue,
                                 OptConstPropagate::TransferFunction>(func->p, false);
    
    Alys::DomTree dom = func_pass_get_dom_ctx(func);
    Alys::LoopInfo loop_info(func->p, dom);

    array_accumulate(func, ans, loop_info);

    func_pass_dse(func);

    func->p.plain_opt_all();
    func->p.re_generate();
}

void function_pass_print(const Ir::pFuncDefined &func)
{
    printf("%s\n", func->print_func().c_str());
}

void pass_inline(const Ir::pModule &mod) {
    mod->remove_unused_function();
    while (inline_all_function(mod))
        ;
}

void pass_dfa(const Ir::pModule &mod) {
    for (auto &&func : mod->funsDefined) {
        int cnt = 0;
        const int MAX_OPT_COUNT = 8;
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = func_pass_dse(func).cnt;
            opt_cnt += func_pass_const_propagate(func).cnt;
        }
        func->p.re_generate();
    }
}

void pass_print(const Ir::pModule &mod) {
    for (auto &&func : mod->funsDefined) {
        func->p.re_generate();
        function_pass_print(func);
    }
}

void optimize(const Ir::pModule &mod) {
    pass_inline(mod);
#ifdef OPT_CONST_PROPAGATE_DEBUG
    pass_print(mod);
#endif
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
        func_pass_canonicalizer(func);
        func_pass_loop_gep_motion(func);
        #ifdef OPT_CONST_PROPAGATE_DEBUG
            function_pass_print(func);
            printf("----------------------------------------------\n");
        #endif
        func_pass_array_accumulate(func);
        func_pass_pointer_iteration(func);
        func_pass_loop_unrolling(func);
        func_pass_gvn(func);
    }
}

} // namespace Optimize
