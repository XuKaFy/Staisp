#pragma once

#include <utility>

#include "def.h"

namespace Backend {
/*
-- LLVM IR --
Instr
    CalculatableInstr
        CasrInstr
            bitcast,
            ptrtoint, inttoptr,
            trunc, sext, zext,
            fptrunc, fpext,
            sitofp, fptosi,
            uitofp, fptoui,
        BinInstr
            add, sub, mul,
            sdiv, udiv, srem, urem,
            fadd, fsub, fmul, fdiv, frem,
            and, or, xor,
            shl, ashr, lshr 
        CmpInstr
            EQ, NE
            UGT, UGE, ULT, ULE
            SGT, SGE, SLT, SLE
            OEQ, UNE
            OGT, OGE, OLT, OLE
        UnaryInstr
            fneg
    BrCondInstr
    BrInstr
    LabelInstr
    AllocInstr
    RetInstr
    PhiInstr
    StoreInstr
    ItemInstr
    CallInstr
    *UnreachableInstr*

-- Machine IR -- RISCV64GC

RISCV 64GC:
IMAFDZicsr_Zifencei
I: Integer
M: Mul & Div
A: Atomics
F: float
D: double
Zicsr: Control and Status Register Access
Zifencei: Instruction-Fetch Fence
C: 16-bit Compressed Instruction

RV64:
    x0=0, x1-x31, PC -> 64bit
    f0-f31 -> width matches the widest precision (64-bit)

MachineInstr
    RTypeInstr
        I:
            sll(RV64I, sllw), srl(RV64I, srlw), sra(RV64I, sraw)
            add(RV64I, addw), sub(RV64I, subw)
            slt, sltu
            xor, or, and
        M:
            mul(RV64I, mulw), mulh
            div(RV64I, divw), divu, rem(RV64I, remw), remu(RV64I, remuw)
        F&D:
            fmv.w.x(RV64I, fmv.d.x) [from int], fmv.x.w(RV64I, fmv.x.d) [to int]
            fcvt.{s|d}.w(RV64I, fcvt.{s|d}.wu), fcvt.{s|d}.l(RV64I, fcvt.{s|d}.lu)
            fcvt.w.{s|d}(RV64I, fcvt.wu.{s|d}), fcvt.l.{s|d}(RV64I, fcvt.lu.{s|d})
            fadd.{s|d}, fsub.{s|d}, fmul.{s|d}, fdiv.{s|d}
            fmadd.{s|d}, fmsub.{s|d}, fnmadd.{s|d}, fnmsub.{s|d}
            fmin.{s|d}, fmax.{s|d}
            feq.{s|d}, flt.{s|d}, fle.{s|d}
    ITypeInstr
        I:
            slli, srli, srai
            addi(RV64I, addiw)
            slti, sltiu
            xori, ori, andi
            lw(RV64I, lwu), lh, lb, lhu, lbu
            (RV64I, ld)
        F&D:
            flw(64, fld)
    STypeInstr
        I:
            sw, sh, sb
            (RV64I, sd)
        F&D:
            fsw(64, fsd)
    BTypeInstr
        I:
            beq, bne, blt, bge
            bltu, bgeu
    UTypeInstr
        I:
            lui
    JTypeInstr
        I:
            jal, jalr
*/

enum class MachineInstrType {
    RTYPE,
    ITYPE,
    STYPE,
    BTYPE,
    UTYPE,
    JTYPE,
    LABEL,
    IMM,
};

struct MachineInstr {
    virtual String instr_print(const std::string& function_name) const = 0;
    virtual MachineInstrType instr_type() const = 0;
};

typedef Pointer<MachineInstr> pMachineInstr;
typedef Vector<pMachineInstr> MachineInstrs;

struct Reg {
    String to_string() const;
    std::string v_reg;
};

enum class RTypeCode {
    SLL, SRL, SRA,
    ADD, SUB, SLT, SLTU,
    XOR, OR, AND,
    MUL, MULH, DIV, DIVU, REM, REMU,
    FMV_W_X, FMV_X_W,
    FCVT_S_W, FCVT_S_WU,
    FCVT_W_S, FCVT_WU_S,
    FADD_S, FSUB_S, FMUL_S, FDIV_S,
    FMADD_S, FMSUB_S, FNMADD_S, FNMSUB_S,
    FMIN_S, FMAX_S,
    FEQ_S, FLT_S, FLE_S
};

struct RTypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::RTYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    RTypeCode code;
    Reg rs1, rs2, rd;
};

enum class ITypeCode {
    SLLI, SRLI, SRAI,
    ADDI, SLTI, SLTIU,
    XORI, ORI, ANDI,
    LW, LH, LB, LHU, LBU,
    FLW
};

struct ITypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::ITYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    ITypeCode code;
    Reg rs, rd;
    int imm;
};

enum class STypeCode {
    SW, SH, SB,
    FSW,
};

struct STypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::STYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    STypeCode code;
    Reg rd, rs;
    int imm;
};

enum class UTypeCode {
    LUI, AUIPC,
};

struct UTypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::UTYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    UTypeCode code;
    Reg rd;
    int imm;
};

enum class BTypeCode {
    BEQ, BNE, BLT, BGE,
    BLTU, BGEU,
};

struct BTypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::BTYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    BTypeCode code;
    Reg rs1, rs2;
    std::string label;
};

enum class JTypeCode {
    JAL, JALR,
};

struct JTypeInstr final : public MachineInstr {
    MachineInstrType instr_type() const override {
        return MachineInstrType::JTYPE;
    }
    String instr_print(const std::string& function_name) const override;
    
    JTypeCode code;
    Reg rd;
    std::string label;
};

} // namespace Backend
