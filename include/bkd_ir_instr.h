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
#include <bkd_freginstrtype.h>
#include <bkd_fregreginstrtype.h>
#include <bkd_fregfreginstrtype.h>
#include <bkd_fcmpinstrtype.h>
#include <bkd_regimmreginstrtype.h>
#include <bkd_fregimmreginstrtype.h>
#include <bkd_reglabelinstrtype.h>
#include <variant>

namespace Backend {
/*
-- LLVM IR --
Instr
    CalculatableInstr
        CastInstr
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

inline std::string stringify(int imm) {
    return std::to_string(imm);
}

inline std::string stringify(std::string value) {
    return value;
}

template<typename Type, typename... Args>
std::string concat(Type&& type, Args&&... args) {
    std::string result;
    result += stringify(type);
    result += " ";
    (((result += stringify(args)) += ", "), ...);
    result.pop_back();
    result.pop_back();
    return result;
}

struct ImmInstr {
    ImmInstrType type;
    Reg rd; int imm;

    std::string stringify() const {
        return concat(type, rd, imm);
    }
};

struct RegInstr {
    RegInstrType type;
    Reg rd, rs;

    std::string stringify() const {
        return concat(type, rd, rs);
    }
};

struct RegRegInstr {
    RegRegInstrType type;
    Reg rd, rs1, rs2;

    std::string stringify() const {
        return concat(type, rd, rs1, rs2);
    }
};

struct RegImmInstr {
    RegImmInstrType type;
    Reg rd, rs; int imm;

    std::string stringify() const {
        return concat(type, rd, rs, imm);
    }
};

struct BranchInstr {
    BranchInstrType type;
    Reg rs1, rs2; std::string label;

    std::string stringify() const {
        return concat(type, rs1, rs2, label);
    }
};

struct FRegInstr {
    FRegInstrType type;
    FReg rd, rs;

    std::string stringify() const {
        return concat(type, rd, rs);
    }
};

struct FRegRegInstr {
    FRegRegInstrType type;
    // note: not in instruction order
    FReg fr; Reg r;

    std::string stringify() const {
        switch (type) {
            case FRegRegInstrType::FCVT_W_S: // convert to   integer
            case FRegRegInstrType::FMV_X_W:  // move    to   integer
                return concat(type, r, fr);
            case FRegRegInstrType::FCVT_S_W: // convert from integer
            case FRegRegInstrType::FMV_W_X:  // move    from integer
                return concat(type, fr, r);
        }
    }
};

struct FRegFRegInstr {
    FRegFRegInstrType type;
    FReg rd, rs1, rs2;

    std::string stringify() const {
        return concat(type, rd, rs1, rs2);
    }
};

struct FCmpInstr {
    FCmpInstrType type;
    Reg rd; FReg rs1, rs2;

    std::string stringify() const {
        return concat(type, rd, rs1, rs2);
    }
};

struct RegImmRegInstr {
    RegImmRegInstrType type;
    Reg rd; int imm; Reg rs;

    std::string stringify() const {
        std::string buf = Backend::stringify(type);
        buf += " ";
        buf += Backend::stringify(rd);
        buf += ", ";
        buf += Backend::stringify(imm);
        buf += "(";
        buf += Backend::stringify(rs);
        buf += ")";
        return buf;
    }
};

struct FRegImmRegInstr {
    FRegImmRegInstrType type;
    FReg rd; int imm; Reg rs;

    std::string stringify() const {
        std::string buf = Backend::stringify(type);
        buf += " ";
        buf += Backend::stringify(rd);
        buf += ", ";
        buf += Backend::stringify(imm);
        buf += "(";
        buf += Backend::stringify(rs);
        buf += ")";
        return buf;
    }
};

struct RegLabelInstr {
    RegLabelInstrType type;
    Reg rd; std::string label;

    std::string stringify() const {
        return concat(type, rd, label);
    }
};

struct JInstr {
    std::string label;

    std::string stringify() const {
        return concat("j", label);
    }
};

struct CallInstr {
    std::string label;

    std::string stringify() const {
        return concat("call", label);
    }
};

struct ReturnInstr {
    std::string stringify() const {
        return "ret";
    }
};


struct MachineInstr {
    enum class Type {
        IMM,
        REG,
        REGREG,
        REGIMM,
        BRANCH,
        FREG,
        FREGREG,
        FREGFREG,
        FCMP,
        REGIMMREG,
        FREGIMMREG,
        REGLABEL,
        J,
        CALL,
        RETURN,
    };

    Type instr_type() const { return (Type) instr.index(); }
    std::string stringify() const {
        return std::visit([](auto&& instr) { return instr.stringify(); }, instr);
    }

    std::variant<
        ImmInstr, RegInstr, RegRegInstr, RegImmInstr,
        BranchInstr, FRegInstr, FRegRegInstr, FRegFRegInstr, FCmpInstr,
        RegImmRegInstr, FRegImmRegInstr, RegLabelInstr,
        JInstr, CallInstr, ReturnInstr
    > instr;
};

typedef Vector<MachineInstr> MachineInstrs;

} // namespace Backend
