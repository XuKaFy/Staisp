#include "alys_loop.h"
#include "ir_func_defined.h"
#include "opt_DFA.h"
#include "opt_const_propagate.h"

namespace Optimize {

void array_accumulate(const Ir::pFuncDefined& func, 
                      const Alys::DomTree &dom,
                      const Alys::LoopInfo &info);

} // namespace Optimize
