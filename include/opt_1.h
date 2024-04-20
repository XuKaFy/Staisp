#pragma once

#include "ir_block.h"
#include "ir_module.h"

namespace Opt1 {

struct Val {
    Val()
        : ty(NAC), v(0) { }
    Val(ImmValue v)
        : ty(VALUE), v(v) { }

    bool operator == (const Val &val) const {
        return v == val.v;
    }

    enum {
        VALUE,
        NAC,
    } ty;
    ImmValue v;
};

struct BlockValue {
    bool operator == (const BlockValue &b);
    bool operator != (const BlockValue &b);

    void cup(const BlockValue &v);

    void clear();

    Map<String, Val> val;
};

struct Utils {
    // transfer function
    void operator () (Ir::pBlock p, BlockValue &v);
};

}; // namespace Opt1
