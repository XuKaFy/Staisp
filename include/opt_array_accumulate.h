#include "alys_loop.h"
#include "ir_func_defined.h"
#include "opt_DFA.h"
#include "opt_const_propagate.h"

namespace Optimize {

void replace_init_with_force(const Ir::pFuncDefined& func, const Alys::LoopInfo &info);

void array_accumulate(const Ir::pFuncDefined& func, 
                      const DFAAnalysisData<OptConstPropagate::BlockValue> &dom,
                      const Alys::LoopInfo &info);

} // namespace Optimize
