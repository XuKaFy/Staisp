#pragma once

#include <utility>

#include "def.h"
#include <bkd_reg.h>
#include <bkd_freg.h>
#include <bkd_imminstrtype.h>
#include <bkd_reginstrtype.h>
#include <bkd_regreginstrtype.h>
#include <bkd_regimminstrtype.h>
#include <bkd_branchinstrtype.h>
#include <bkd_jumpinstrtype.h>
#include <bkd_freginstrtype.h>
#include <bkd_fregreginstrtype.h>
#include <bkd_fregfreginstrtype.h>

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
    //
};

struct MachineInstr {
    virtual String instr_print(const std::string& function_name) const = 0;
    virtual MachineInstrType instr_type() const = 0;
};

typedef Pointer<MachineInstr> pMachineInstr;
typedef Vector<pMachineInstr> MachineInstrs;

} // namespace Backend
