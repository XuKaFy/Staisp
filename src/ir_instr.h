#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

enum InstrType {
    INSTR_TYPE_BIN,
    INSTR_TYPE_UNARY,
};

#define IMM_TYPE_TABLE \
    ENTRY(1, 8) \
    ENTRY(2, 16) \
    ENTRY(3, 32) \
    ENTRY(4, 64)

enum ImmType {
#define ENTRY(x, y) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

#define ENTRY(x, y) "i" #y,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY

#undef IMM_TYPE_TABLE

#define UNARY_INSTR_TABLE \
    ENTRY(NEG, neg) \
    ENTRY(NOT, not) \
    ENTRY(MOV, mov)

#define BIN_INSTR_TABLE \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(MUL, mul) \
    ENTRY(SDIV, sdiv) \
    ENTRY(REM, rem) \
    ENTRY(CALL, call) \
    ENTRY(AND, and) \
    ENTRY(OR, or) \
    ENTRY(ALLOC, alloc) \
    ENTRY(LOAD, load) \
    ENTRY(STORE, store) \
    ENTRY(BR, br) \
    ENTRY(RET, ret)

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

struct Instr : public Val {
    Instr(InstrType instrType, ImmType tr)
        : instrType(instrType), tr(tr) { }
    InstrType instrType;
    ImmType tr;
};

typedef Pointer<Instr> pInstr;

struct UnaryInstr : public Instr {
    UnaryInstr(UnaryInstrType unaryType, ImmType tr, pVal oprd)
        : Instr(INSTR_TYPE_UNARY, tr), unaryType(unaryType), oprd(oprd) { }

    virtual Symbol print_impl() const override;
    UnaryInstrType unaryType;
    pVal oprd;
};

struct BinInstr : public Instr {
    BinInstr(BinInstrType binType, ImmType tr, pVal oprd[2])
        : Instr(INSTR_TYPE_BIN, tr), binType(binType), oprd{oprd[0], oprd[1]} { }

    virtual Symbol print_impl() const override;
    BinInstrType binType;
    pVal oprd[2];
};

pInstr make_unary_instr(UnaryInstrType type, ImmType tr, pVal oprd);
pInstr make_binary_instr(BinInstrType type, ImmType tr, pVal oprd1, pVal oprd2);

} // namespace Ir
