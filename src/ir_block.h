#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

struct Block : public Val {
    virtual Symbol print_impl() const override;
    pInstr add_instr(pInstr instr);
    Vector<pInstr> instrs;
};

} // namespace Ir
