#include "bkd_ir_instr.h"
#include "def.h"
#include <string>

namespace Backend {

String RegPos::to_string() const
{
    if (reg_id == -1) {
        my_assert(instr, "?");
        return instr->name();
    }
    if (reg_id == -2) {
        return std::to_string(imm);
    }
    return "x" + std::to_string(reg_id);
}

String RTypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case RTypeCode::ADD:            name = "add"; break;
    case RTypeCode::AND:            name = "and"; break;
    case RTypeCode::DIV:            name = "div"; break;
    case RTypeCode::DIVU:           name = "divu"; break;
    case RTypeCode::FADD_S:         name = "fadd.s"; break;
    case RTypeCode::FCVT_S_W:       name = "fcvt.s.w"; break;
    case RTypeCode::FCVT_S_WU:      name = "fcvt.s.wu"; break;
    case RTypeCode::FCVT_W_S:       name = "fcvt.w.s"; break;
    case RTypeCode::FCVT_WU_S:      name = "fcvt.wu.s"; break;
    case RTypeCode::FDIV_S:         name = "fdiv.s"; break;
    case RTypeCode::FEQ_S:          name = "feq.s"; break;
    case RTypeCode::FLT_S:          name = "flt.s"; break;
    case RTypeCode::FLE_S:          name = "fle.s"; break;
    case RTypeCode::FMADD_S:        name = "fmadd.s"; break;
    case RTypeCode::FMAX_S:         name = "fmax.s"; break;
    case RTypeCode::FMIN_S:         name = "fmin.s"; break;
    case RTypeCode::FMSUB_S:        name = "fmsub.s"; break;
    case RTypeCode::FMUL_S:         name = "fmul.s"; break;
    case RTypeCode::FNMADD_S:       name = "fnmadd.s"; break;
    case RTypeCode::FNMSUB_S:       name = "fnmsub.s"; break;
    case RTypeCode::FSUB_S:         name = "fsub.s"; break;
    case RTypeCode::FMV_W_X:        name = "fmv.w.x"; break;
    case RTypeCode::FMV_X_W:        name = "fmv.x.w"; break;
    case RTypeCode::MUL:            name = "mul"; break;
    case RTypeCode::MULH:           name = "mulh"; break;
    case RTypeCode::OR:             name = "or"; break;
    case RTypeCode::REM:            name = "rem"; break;
    case RTypeCode::REMU:           name = "remu"; break;
    case RTypeCode::SLL:            name = "sll"; break;
    case RTypeCode::SLT:            name = "slt"; break;
    case RTypeCode::SLTU:           name = "slltu"; break;
    case RTypeCode::SRA:            name = "sra"; break;
    case RTypeCode::SRL:            name = "srl"; break;
    case RTypeCode::SUB:            name = "sub"; break;
    case RTypeCode::XOR:            name = "xor"; break; 
    }
    return name + " " + rd.to_string() + ", " + rs1.to_string() + ", " + rs2.to_string();
}

String ITypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case ITypeCode::SLLI:       name = "slli"; break;
    case ITypeCode::SRLI:       name = "srli"; break;
    case ITypeCode::SRAI:       name = "srai"; break;
    case ITypeCode::ADDI:       name = "addi"; break;
    case ITypeCode::SLTI:       name = "slti"; break;
    case ITypeCode::SLTIU:      name = "sltiu"; break;
    case ITypeCode::XORI:       name = "xori"; break;
    case ITypeCode::ORI:        name = "ori"; break;
    case ITypeCode::ANDI:       name = "andi"; break;
    case ITypeCode::LW:         name = "lw"; break;
    case ITypeCode::LH:         name = "lh"; break;
    case ITypeCode::LB:         name = "lb"; break;
    case ITypeCode::LHU:        name = "lhu"; break;
    case ITypeCode::LBU:        name = "lbu"; break;
    case ITypeCode::FLW:        name = "flw"; break;
    }
    return name + " " + rd.to_string() + ", " + rs.to_string() + ", " + std::to_string(imm);
}

String STypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case STypeCode::SW:        name = "sw"; break;
    case STypeCode::SH:        name = "sh"; break;
    case STypeCode::SB:        name = "sb"; break;
    case STypeCode::FSW:       name = "fsw"; break;
    }
    return name + " " + rd.to_string() + ", " + rs.to_string() + ", " + std::to_string(imm);
}

String BTypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case BTypeCode::BEQ:    name = "beq"; break;
    case BTypeCode::BNE:    name = "bne"; break;
    case BTypeCode::BLT:    name = "blt"; break;
    case BTypeCode::BLTU:   name = "bltu"; break;
    case BTypeCode::BGE:    name = "bge"; break;
    case BTypeCode::BGEU:   name = "bgeu"; break;
    }
    return name + " " + rs1.to_string() + ", " + rs2.to_string() + ", " + std::to_string(imm);
}

String JTypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case JTypeCode::JAL:   name = "jal"; break;
    case JTypeCode::JALR:  name = "jalr"; break;
    }
    return name + " " + rd.to_string() + ", " + std::to_string(imm);
}

String UTypeInstr::instr_print() const
{
    String name;
    switch (code) {
    case UTypeCode::LUI:    name = "lui"; break;
    case UTypeCode::AUIPC:  name = "auipc"; break;
    }
    return name + " " + rd.to_string() + ", " + std::to_string(imm);
}

} // namespace Backend
