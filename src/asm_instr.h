#pragma once

#include "def.h"
#include "asm_regs.h"

namespace Asm {

#define Instr_AND_NAME \
    ENTRY(LUI, lui) \
    ENTRY(BEQ, beq) \
    ENTRY(BNE, bne) \
    ENTRY(BLT, blt) \
    ENTRY(BGE, bge) \
    ENTRY(LW, lw) \
    ENTRY(SW, sw) \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(OR, or) \
    ENTRY(XOR, xor) \
    ENTRY(AND, and) \
    ENTRY(ADDW, addw) \
    ENTRY(SUBW, subw) \
    ENTRY(MUL, mul) \
    ENTRY(DIV, div) \
    ENTRY(REM, rem) \
    ENTRY(MULW, mulw) \
    ENTRY(DIVW, divw) \
    ENTRY(REMW, remw) \

enum InstrType
{
#define ENTRY(x, y) INST_##x,
    Instr_AND_NAME
#undef ENTRY
};

const Symbol InsName[32] {
#define ENTRY(x, y) Symbol(#y),
    Instr_AND_NAME
#undef ENTRY
};

#undef Instr_AND_NAME

struct Instr
{
    Symbol to_string() const;

    InstrType type;
    Immediate imm, imm1, imm2, imm3;
    pReg rs1, rs2, rd;
};

} // namespace asm
