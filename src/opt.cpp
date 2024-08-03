#include "opt.h"

#include <cstdlib>
#include <reg2mem.h>

#include "alys_dom.h"
#include "alys_loop.h"
#include "def.h"
#include "opt_DFA.h"
#include "opt_DSE.h"
#include "opt_const_propagate.h"
#include "opt_interprocedural.h"
#include "trans_SSA.h"
#include "trans_loop.h"

extern int flag_O1;

namespace Optimize {

void optimize(const Ir::pModule &mod, AstToIr::Convertor &convertor) {
#ifndef OPT_CONST_PROPAGATE_DEBUG
    mod->remove_unused_function();
    while (inline_all_function(mod, convertor));
    mod->remove_unused_function();
#endif
    for (auto &&func : mod->funsDefined) {
        from_top_analysis<OptConstPropagate::BlockValue,
                          OptConstPropagate::TransferFunction>(func->p);
        func->p.plain_opt_all();
        my_assert(func->p.check_empty_use("CP") == 0, "CP Failed");
    }
    global2local(mod);
    for (auto &&func : mod->funsDefined) {
        SSA_pass pass(func->p);
        pass.reconstruct();
        func->p.plain_opt_all();
        my_assert(func->p.check_empty_use("SSA") == 0, "SSA Failed");
#ifdef OPT_CONST_PROPAGATE_DEBUG
        func->p.re_generate();
        printf("BEFORE\n%s\n", func->print_func().c_str());
#endif
        int cnt = 0;
        func->p.re_generate(); // must re-generate for DFA
        const int MAX_OPT_COUNT = flag_O1 ? 8 : 2;
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = from_bottom_analysis<OptDSE::BlockValue,
                                           OptDSE::TransferFunction>(func->p);
            func->p.plain_opt_all();
            my_assert(func->p.check_empty_use("DSE") == 0, "DSE Failed");
            opt_cnt +=
                from_top_analysis<OptConstPropagate::BlockValue,
                                  OptConstPropagate::TransferFunction>(func->p);
            func->p.plain_opt_all();
            my_assert(func->p.check_empty_use("CP") == 0, "CP Failed");

#ifdef OPT_CONST_PROPAGATE_DEBUG
            func->p.re_generate();
            printf("AFTER %d\n%s\n", cnt, func->print_func().c_str());
#endif
        }

        Alys::DomTree dom_ctx;
        dom_ctx.build_dom(func->p);
        Alys::LoopInfo loop_info(func->p, dom_ctx);
        Optimize::Canonicalizer_pass(func->p, dom_ctx).ap();
        my_assert(func->p.check_empty_use("Canonicalizer") == 0,
                  "Canonicalizer Failed");

        func->p.re_generate();
        Optimize::LoopGEPMotion_pass(func->p, dom_ctx).ap();
        my_assert(func->p.check_empty_use("LoopGEPMotion") == 0,
                  "LoopGEPMotion Failed");
        // printf("%s\n", i->print_func().c_str());

        // // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name().c_str(), cnt);
        func->p.re_generate();
    }
    // postponed to backend
    // for (auto &&func : mod->funsDefined) {
    //     reg2mem(func->p);
    //     func->p.re_generate();
    // }
}

} // namespace Optimize
