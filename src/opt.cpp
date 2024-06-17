#include "opt.h"

#include "trans_SSA.h"
#include "opt_interprocedural.h"
#include "opt_const_propagate.h"
#include "opt_DSE.h"
#include "opt_DFA.h"

#define MAX_OPT_COUNT 1

namespace Optimize {

void optimize(const Ir::pModule &mod, AstToIr::Convertor &convertor) {
    inline_all_function(mod, convertor);
    global2local(mod);
    for (auto &&func : mod->funsDefined) {
        // printf("%s\n", i->print_func().c_str());
        int cnt = 0;
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = from_bottom_analysis<OptDSE::BlockValue, OptDSE::TransferFunction>(func->p);
            func->p.plain_opt_all();
            opt_cnt += from_top_analysis<OptConstPropagate::BlockValue, OptConstPropagate::TransferFunction>(func->p);
            func->p.plain_opt_all();
        }
        SSA_pass pass(func->p, ssa_type::RECONSTRUCTION);
        pass.pass_transform();
        // DO NOT EDIT: OPTIMIZE TWICE AS INTENDED
        func->p.plain_opt_no_bb();
        func->p.plain_opt_no_bb();

        // // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name().c_str(), cnt);
        func->p.re_generate();
    }
}

}
