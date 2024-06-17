#pragma once

#include "ir_block.h"

namespace OptConstPropagate {

struct Val {
    Val() : ty(NAC), v(0) {}
    Val(ImmValue v) : ty(VALUE), v(v) {}

    bool operator==(const Val &val) const {
        if (ty != val.ty) {
            return false;
        }
        if (ty == NAC) {
            return true;
        }
        return v == val.v;
    }

    bool operator!=(const Val &val) const { return !operator==(val); }

    enum {
        VALUE,
        NAC,
    } ty;
    ImmValue v;
    Ir::pInstr ir;
};

struct BlockValue {
    bool operator==(const BlockValue &b) const { return val == b.val; }
    bool operator!=(const BlockValue &b) const { return !operator==(b); }

    void cup(const BlockValue &v);

    void clear() { val.clear(); }

    Map<String, Val> val;
};

struct TransferFunction {
    void operator()(Ir::Block *p, BlockValue &v);
    int operator()(Ir::Block *p, const BlockValue &IN, const BlockValue &OUT);
};

}
