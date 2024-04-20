#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#define UNARY_INSTR_TABLE \
    ENTRY(NEG, neg) \
    ENTRY(NOT, not)

#define BIN_INSTR_TABLE \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(MUL, mul) \
    ENTRY(DIV, div) \
    ENTRY(REM, rem) \
    ENTRY(AND, and) \
    ENTRY(OR, or)

#define ENTRY(x, y) INSTR_##x,
enum UnaryInstrType {
    UNARY_INSTR_TABLE
};
enum BinInstrType {
    BIN_INSTR_TABLE
};
#undef ENTRY

#define ENTRY(x, y) #y,
const Symbol gUnaryInstrName[] = {
    UNARY_INSTR_TABLE
};
const Symbol gBinInstrName[] = {
    BIN_INSTR_TABLE
};
#undef ENTRY

#undef UNARY_INSTR_TABLE
#undef BIN_INSTR_TABLE

struct UnaryInstr : public CalculatableInstr {
    UnaryInstr(UnaryInstrType unaryType, pVal oprd)
        : CalculatableInstr(oprd->ty), unaryType(unaryType) {
        add_operand(oprd);
    }

    virtual Symbol instr_print_impl() const override;

    virtual InstrType instr_type() const override {
        return INSTR_UNARY;
    }

    virtual ImmValue calculate(Vector<ImmValue> v) const override;

    UnaryInstrType unaryType;
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

pInstr make_unary_instr(UnaryInstrType type, pVal oprd);
pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2);

} // namespace ir
