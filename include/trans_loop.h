
#pragma once

#include "alys_dom.h"
#include "ir_block.h"
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
} // namespace Optimize