// 优化二：活跃变量分析，删除无用 store

#pragma once

#include "ir_block.h"

namespace Opt2 {

struct BlockValue {
    bool operator==(const BlockValue &b);
    bool operator!=(const BlockValue &b);

    void cup(const BlockValue &v);

    void clear();

    Set<String> uses;
};

struct Utils {
    // transfer function
    void operator()(Ir::Block *p, BlockValue &v);
    int operator()(Ir::Block *p, const BlockValue &IN, const BlockValue &OUT);
};

} // namespace Opt2
