#pragma once

#include "bkd_block.h"
#include "def.h"

namespace Backend {

struct Func {
    Vector<pBlock> body;
    String name;
};

typedef Pointer<Func> pFunc;

} // namespace Backend
