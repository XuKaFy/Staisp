#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#define BIN_INSTR_TABLE \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(MUL, mul) \
    ENTRY(DIV, div) \
    ENTRY(REM, rem) \
    ENTRY(AND, and) \
    ENTRY(OR, or)

#define ENTRY(x, y) INSTR_##x,
enum BinInstrType {
    BIN_INSTR_TABLE
};
#undef ENTRY

#define ENTRY(x, y) #y,
const Symbol gBinInstrName[] = {
    BIN_INSTR_TABLE
};
#undef ENTRY

#undef BIN_INSTR_TABLE

struct UnaryInstr : public CalculatableInstr {
    UnaryInstr(pVal oprd)
        : CalculatableInstr(oprd->ty) {
        add_operand(oprd);
    }

    virtual Symbol instr_print_impl() const override;

    virtual InstrType instr_type() const override {
        return INSTR_UNARY;
    }

    virtual ImmValue calculate(Vector<ImmValue> v) const override;

};

struct BinInstr : public CalculatableInstr {
    BinInstr(BinInstrType binType, pVal oprd1, pVal oprd2)
        : CalculatableInstr(oprd1->ty), binType(binType) {
        add_operand(oprd1);
        add_operand(oprd2);
    }

    virtual Symbol instr_print_impl() const override;

    virtual ImmValue calculate(Vector<ImmValue> v) const override;

    virtual InstrType instr_type() const override {
        return INSTR_BINARY;
    }

    BinInstrType binType;
};

pInstr make_unary_instr(pVal oprd);
pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2);

} // namespace ir
