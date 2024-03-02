#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

struct Block : public Val {
    virtual Symbol print_impl() const override;

    Symbol print_block(size_t line = 0) const;

    pInstr add_instr(pInstr instr);
    Vector<pInstr> instrs;
};

typedef Pointer<Block> pBlock;

pBlock make_block();

} // namespace Ir
