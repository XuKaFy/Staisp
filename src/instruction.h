#pragma once

#include "def.h"
#include "regs.h"

#define INSTRUCTION_AND_NAME \
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

enum InstructionType
{
#define ENTRY(x, y) INST_##x,
    INSTRUCTION_AND_NAME
#undef ENTRY
};

const Symbol InsName[32] {
#define ENTRY(x, y) Symbol(#y),
    INSTRUCTION_AND_NAME
#undef ENTRY
};

#undef INSTRUCTION_AND_NAME

struct Instruction
{
    Symbol to_string() const;

    InstructionType type;
    Immediate imm, imm1, imm2, imm3;
    pReg rs1, rs2, rd;
};
