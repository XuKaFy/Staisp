#pragma once

#include "def.h"

#include "ir_instr.h"
#include "ir_control_instr.h"
#include "ir_call_instr.h"
#include "ir_line_generator.h"

namespace Ir {

struct Block;
typedef Pointer<Block> pBlock;
typedef WeakPointer<Block> pwBlock;
typedef Vector<pBlock> Blocks;

struct Block : public Val {
    Block()
        : Val(make_void_type()) { }

    Symbol print_block() const;
    
    pInstr label() const;
    pInstr back() const;

    void push_back(pInstr instr);
    void connect(Block* next);

    Instrs body;

    Vector<Block*> in_block;
    Vector<Block*> out_block;
};

struct BlockedProgram
{
    void from_instrs(const Instrs &instrs);
    void push_back(pInstr instr);

    Blocks blocks;
};

pBlock make_block();

} // namespace Ir
