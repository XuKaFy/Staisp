#pragma once

#include <utility>

#include "ir_instr.h"

namespace Ir {

enum CastMethod {
    CAST_BITCAST,
    CAST_PTRTOINT,
    CAST_INTTOPTR,
    CAST_TRUNC,
    CAST_SEXT,
    CAST_ZEXT,
    CAST_FPEXT,
    CAST_FPTRUNC,
    CAST_SITOFP,
    CAST_UITOFP,
    CAST_FPTOSI,
    CAST_FPTOUI,
};

const String CAST_METHOD_NAME[] = {
    "bitcast",
    "ptrtoint",
    "inttoptr",
    "trunc",
    "sext",
    "zext",
    "fpext",  
    "fptrunc",
    "sitofp",
    "fptosi",
    "fptosi",
    "fptoui",
};

CastMethod get_cast_method(pType from, pType to);

struct CastInstr : public CalculatableInstr {
    CastInstr(pType tr, Val* a1) : CalculatableInstr(std::move(tr)), _method(get_cast_method(a1->ty, ty)) {
        add_operand(a1);
    }

    InstrType instr_type() const override { return INSTR_CAST; }

    String instr_print() const override;

    ImmValue calculate(Vector<ImmValue> v) const override;

    CastMethod method() const {
        return _method;
    }

    Instr* clone_internal() const override {
        return new CastInstr(ty, operands()[0]->usee);
    }

private:

    CastMethod _method;
};

pInstr make_cast_instr(pType ty, Val* a1);

} // namespace Ir
