#pragma once

#include "def.h"
#include "regs.h"

enum InstructionType
{
    INSTRUCTION_UNKNOWN,
    INSTRUCTION_MUL,
    INSTRUCTION_ADD,
    INSTRUCTION_MOV32,
    INSTRUCTION_MOV64,
    INSTRUCTION_CALL,
    INSTRUCTION_RET,
};

struct Instruction
{
    InstructionType type;
    Reg r[2];
    Integer val;
};

struct Chunk
{
    void push_mov(Reg left, Reg right);
    void push_mov(Reg left, Integer right);
    void push_opr(InstructionType ins, Reg l, Reg r);
    void print() const;
    Vector<Instruction> instructions;
};
