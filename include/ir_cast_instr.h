#pragma once

#include "ir_instr.h"

namespace Ir {

struct CastInstr : public CalculatableInstr {
    CastInstr(pType tr, pVal a1) : CalculatableInstr(tr) { add_operand(a1); }

    virtual InstrType instr_type() const override { return INSTR_CAST; }

    virtual String instr_print() const override;

    virtual ImmValue calculate(Vector<ImmValue> v) const override;

private:
    String use(String inst) const;
};

pInstr make_cast_instr(pType tr, pVal a1);

} // namespace Ir
