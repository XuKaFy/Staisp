#pragma once

#include "ir_instr.h"

namespace Ir
{

struct CastInstr : public Instr {
    CastInstr(ImmType tr, pInstr a1)
        : Instr(INSTR_TYPE_NEED_REG, tr), val(a1) { }

    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }

    pInstr val;
};

pInstr make_cast_instr(ImmType tr, pInstr a1);

} // namespace ir
