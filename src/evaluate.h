#pragma once

#include "def.h"
#include "node.h"
#include "regs.h"
#include "chunk.h"

Reg evaluate(Chunk &c, pNode n, Regs &regs);
