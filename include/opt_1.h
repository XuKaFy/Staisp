// 优化一：常量传播，将常量进行计算和替换

#pragma once

#include "ir_block.h"

namespace Opt1 {

struct Val {
    Val()
        : ty(NAC), v(0) { }
    Val(ImmValue v)
        : ty(VALUE), v(v) { }

    bool operator == (const Val &val) const {
        return v == val.v;
    }
    
    bool operator != (const Val &val) const {
        return v != val.v;
    }

    enum {
        VALUE,
        NAC,
    } ty;
    ImmValue v;
    Ir::pInstr ir;
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
    void operator () (Ir::Block* p, BlockValue &v);
    int operator () (Ir::Block* p, const BlockValue &IN, const BlockValue &OUT);
};

}; // namespace Opt1
