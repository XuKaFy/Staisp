#include "asm_instr.h"

namespace Asm {

Symbol Instr::to_string() const
{
    switch(type) {
    // R-type rs2, rs1, rd
    case INST_ADD:
    case INST_SUB:
    case INST_OR:
    case INST_XOR:
    case INST_AND:
    case INST_ADDW:
    case INST_SUBW:
    case INST_MUL:
    case INST_DIV:
    case INST_REM:
    case INST_MULW:
    case INST_DIVW:
    case INST_REMW:
        return to_symbol(String(gInstrName[type]) + "\t" + rd->name + "," + rs1->name + "," + rs2->name);
    // I-type imm[11:0], rs1, rd
    // S-type imm[11:5], rs2, rs1, imm[4:0]
    // U-type imm[31:12], rd
    default: break;
    }
    return "";
}

}  // namespace asm

