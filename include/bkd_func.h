#pragma once

#include "bkd_block.h"
#include "def.h"

namespace Backend {

struct Func {
    String print() const;

    Vector<Block> body;
};

} // namespace Backend
