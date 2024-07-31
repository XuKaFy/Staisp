#pragma once

#include <utility>

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#define BIN_INSTR_TABLE                                                        \
    ENTRY(ADD, add)                                                            \
    ENTRY(SUB, sub)                                                            \
    ENTRY(MUL, mul)                                                            \
    ENTRY(FADD, fadd)                                                          \
    ENTRY(FSUB, fsub)                                                          \
    ENTRY(FMUL, fmul)                                                          \
    ENTRY(SDIV, sdiv)                                                          \
    ENTRY(SREM, srem)                                                          \
    ENTRY(UDIV, udiv)                                                          \
    ENTRY(UREM, urem)                                                          \
    ENTRY(FDIV, fdiv)                                                          \
    ENTRY(FREM, frem)                                                          \
    ENTRY(XOR, xor)                                                            \
    ENTRY(AND, and)                                                            \
    ENTRY(OR, or)                                                              \
    ENTRY(ASHR, ashr)                                                          \
    ENTRY(LSHR, lshr)                                                          \
    ENTRY(SHL, shl)                                                            \
    ENTRY(SLT, slt) /* magic binary op for backend convenience*/

#define ENTRY(x, y) INSTR_##x,
enum BinInstrType { BIN_INSTR_TABLE };
#undef ENTRY

#define ENTRY(x, y) #y,
const String gBinInstrName[] = {BIN_INSTR_TABLE};
#undef ENTRY

#undef BIN_INSTR_TABLE

struct UnaryInstr : public CalculatableInstr {
    UnaryInstr(const pVal &oprd) : CalculatableInstr(oprd->ty) {
        add_operand(oprd);
    }

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_UNARY; }

    ImmValue calculate(Vector<ImmValue> v) const override;
};

struct BinInstr : public CalculatableInstr {
    BinInstr(BinInstrType binType, Val* oprd1, Val* oprd2)
        : CalculatableInstr(oprd1->ty), binType(binType) {
        add_operand(oprd1);
        add_operand(oprd2);
    }

    String instr_print() const override;

    ImmValue calculate(Vector<ImmValue> v) const override;

    InstrType instr_type() const override { return INSTR_BINARY; }

    BinInstrType binType;
};

pInstr make_unary_instr(const pVal &oprd);
pInstr make_binary_instr(BinInstrType type, const pVal &oprd1,
                         const pVal &oprd2);

} // namespace Ir
