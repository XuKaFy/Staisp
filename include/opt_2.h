#pragma once

#include "ir_block.h"
#include "ir_module.h"

#include "ir_ptr_instr.h"

namespace Opt2 {

struct BlockValue {
    bool operator == (const BlockValue &b);
    bool operator != (const BlockValue &b);

    void cup(const BlockValue &v);

    void clear();

    Set<String> uses;
};

struct Utils {
    // transfer function
    void operator () (Ir::Block* p, BlockValue &v);
    void operator () (Ir::Block* p, const BlockValue &IN, const BlockValue &OUT);
};

} // namespace Optimize
