#pragma once

#include "def.h"

#include "ir_instr.h"
#include "ir_control_instr.h"

namespace Ir {

struct Block;
typedef Pointer<Block> pBlock;

struct Block : public Val {
    Block() {
        add_instr(make_label_instr());
    }

    virtual Symbol print_impl() const override;
    
    Symbol print_block() const;
    void generate_line(size_t &line) const;

    pInstr label() const;
    pInstr add_instr(pInstr instr);
    void finish_block_with_jump(pBlock b);
    Vector<pInstr> instrs;
};

pBlock make_block();

} // namespace Ir
