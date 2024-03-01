#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

enum ImmType {
    IMM_8 = 1,
    IMM_16 = 2,
    IMM_32 = 4,
    IMM_64 = 8,
};

enum BinInstrType {
    INSTR_ADD,
    INSTR_SUB,
    INSTR_MUL,
    INSTR_SDIV, // signed division
    INSTR_REM,
    INSTR_CALL,
    INSTR_AND,
    INSTR_OR,
    INSTR_ALLOC,
    INSTR_LOAD,
    INSTR_STORE,
    INSTR_BR,
    INSTR_RET
};

enum UnaryInstrType {
    INSTR_NEG,
    INSTR_NOT,
    INSTR_MOV, // why ??
};

enum InstrType {
    INSTR_TYPE_BIN,
    INSTR_TYPE_UNARY,
};

struct Instr {
    InstrType type;
};

typedef Pointer<Instr> pInstr;

struct BinInstr : public Instr {
    BinInstrType binType;
    ImmType tr;
    pVal oprd[2];
};

struct UnaryInstr : public Instr {
    UnaryInstrType unaryType;
    ImmType tr;
    pVal oprd;
};

} // namespace Ir
