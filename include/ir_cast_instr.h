#pragma once

#include "ir_instr.h"

namespace Ir
{

struct CastInstr : public Instr {
    CastInstr(pType tr, pVal a1)
        : Instr(tr) {
        add_operand(a1);
    }

    virtual Symbol instr_print_impl() const override;

private:
    Symbol use(Symbol inst) const;
};

pInstr make_cast_instr(pType tr, pVal a1);

} // namespace ir
