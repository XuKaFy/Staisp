
#pragma once

#include "alys_dom.h"
#include "ir_block.h"
namespace Optimize {
class Canonicalizer_pass {
    Ir::BlockedProgram &cur_func;
    Alys::DomTree &dom_ctx;
    void ap();
    void rewrite_induction();
};
} // namespace Optimize