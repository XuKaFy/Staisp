#pragma once

#include "ir_block.h"

namespace OptDSE {

struct BlockValue {
    bool operator==(const BlockValue &b) const;
    bool operator!=(const BlockValue &b) const;

    void cup(const BlockValue &v);

    void clear();

    Set<String> uses;
};

struct TransferFunction {
    void operator()(Ir::Block *p, BlockValue &v);
    int operator()(Ir::Block *p, const BlockValue &IN, const BlockValue &OUT);
};

}
