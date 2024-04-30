#pragma once

#include "def.h"

#include "ir_instr.h"
#include "ir_control_instr.h"
#include "ir_call_instr.h"
#include "ir_line_generator.h"

namespace Ir {

struct Block;
typedef Pointer<Block> pBlock;
typedef Vector<pBlock> Blocks;

struct Block : public Val {
    Block()
        : Val(make_void_type()) { }

    virtual ~Block() {
        for(auto i : in_block) {
            i->out_block.erase(this);
        }
        for(auto i : out_block) {
            i->in_block.erase(this);
        }
    }

    void connect_in_and_out();
    void squeeze_out(bool selected);
    void replace_out(Block* before, Block* out);

    Symbol print_block() const;
    
    pInstr label() const;
    pInstr back() const;

    void add_imm(pVal imm);

    void push_back(pInstr instr);
    void connect(Block* next);

    Instrs body;

    virtual ValType type() const {
        return VAL_BLOCK;
    }

    Set<Block*> in_block;
    Set<Block*> out_block;
    Vector<pVal> imms;
};

struct BlockedProgram
{
    void from_instrs(Instrs &instrs);
    void push_back(pInstr instr);
    void re_generate() const;
    void generate_cfg() const;

    void print_cfg() const;

    void join_blocks();

    Blocks blocks;
};

pBlock make_block();

} // namespace Ir
