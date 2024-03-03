#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

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

struct UnaryInstr : public Instr {
    UnaryInstr(UnaryInstrType unaryType, ImmType tr, pVal oprd)
        : Instr(INSTR_TYPE_NEED_REG, tr), unaryType(unaryType), oprd(oprd) { }

    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }
    UnaryInstrType unaryType;
    pVal oprd;
};

struct BinInstr : public Instr {
    BinInstr(BinInstrType binType, ImmType tr, pVal oprd1, pVal oprd2)
        : Instr(INSTR_TYPE_NEED_REG, tr), binType(binType), oprd{oprd1, oprd2} { }

    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }
    BinInstrType binType;
    pVal oprd[2];
};

pInstr make_unary_instr(UnaryInstrType type, ImmType tr, pVal oprd);
pInstr make_binary_instr(BinInstrType type, ImmType tr, pVal oprd1, pVal oprd2);

} // namespace ir
