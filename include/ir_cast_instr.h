#pragma once

#include <utility>

#include "ir_instr.h"

namespace Ir {

struct CastInstr : public CalculatableInstr {
    CastInstr(pType tr, const pVal &a1) : CalculatableInstr(std::move(tr)) {
        add_operand(a1);
    }

    InstrType instr_type() const override { return INSTR_CAST; }

    String instr_print() const override;

    ImmValue calculate(Vector<ImmValue> v) const override;

private:
    String use(const String &inst) const;
};

pInstr make_cast_instr(pType ty, const pVal &a1);

} // namespace Ir
