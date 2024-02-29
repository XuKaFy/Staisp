#pragma once

#include "def.h"
#include "node.h"
#include "regs.h"
#include "chunk.h"

InstructionType convertTypeFromNodeToInstruction(OperationType t);

Reg evaluate(Chunk &c, Pointer<Node> n, Regs &regs);
