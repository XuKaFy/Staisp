#pragma once

#include "def.h"
#include "bkd_ir_instr.h"

namespace Backend {

struct Block {
    Vector<pMachineInstr> body;
};

typedef Pointer<Block> pBlock;

} // namespace Backend
