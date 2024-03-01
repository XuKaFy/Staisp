#pragma once

#include "def.h"
#include "regs.h"
#include "instruction.h"

struct Chunk
{
    Vector<Instruction> instructions;
};
