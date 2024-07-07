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
#include <bkd_regfreginstrtype.h>
#include <bkd_fregfreginstrtype.h>
#include <bkd_fcmpinstrtype.h>
#include <bkd_regimmreginstrtype.h>
#include <bkd_fregimmreginstrtype.h>
#include <bkd_reglabelinstrtype.h>
#include <variant>

namespace Backend {

inline std::string stringify(int imm) {
    return std::to_string(imm);
}

inline std::string stringify(std::string value) {
    return value;
}

template<typename Type, typename... Args>
std::string format(Type&& type, Args&&... args) {
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
        return format(type, rd, imm);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct RegInstr {
    RegInstrType type;
    Reg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {rs}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct RegRegInstr {
    RegRegInstrType type;
    Reg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {rs1, rs2}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct RegImmInstr {
    RegImmInstrType type;
    Reg rd, rs; int imm;

    std::string stringify() const {
        return format(type, rd, rs, imm);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {rs}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct BranchInstr {
    BranchInstrType type;
    Reg rs1, rs2; std::string label;

    std::string stringify() const {
        return format(type, rs1, rs2, label);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {rs1, rs2}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct FRegInstr {
    FRegInstrType type;
    FReg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {rd}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct FRegRegInstr {
    FRegRegInstrType type;
    FReg rd; Reg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {rs}; }
    std::vector<FReg> fdef() const { return {rd}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct RegFRegInstr {
    RegFRegInstrType type;
    Reg rd; FReg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {rd}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {rs}; }
};

struct FRegFRegInstr {
    FRegFRegInstrType type;
    FReg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {rd}; }
    std::vector<FReg> fuse() const { return {rs1, rs2}; }
};

struct FCmpInstr {
    FCmpInstrType type;
    Reg rd; FReg rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {rs1, rs2}; }
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

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {rs}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
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

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {rs}; }
    std::vector<FReg> fdef() const { return {rd}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct RegLabelInstr {
    RegLabelInstrType type;
    Reg rd; std::string label;

    std::string stringify() const {
        return format(type, rd, label);
    }

    std::vector<Reg> def() const { return {rd}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct JInstr {
    std::string label;

    std::string stringify() const {
        return format("j", label);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct CallInstr {
    std::string label;

    std::string stringify() const {
        return format("call", label);
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
};

struct ReturnInstr {
    std::string stringify() const {
        return "ret";
    }

    std::vector<Reg> def() const { return {}; }
    std::vector<Reg> use() const { return {Reg::RA}; }
    std::vector<FReg> fdef() const { return {}; }
    std::vector<FReg> fuse() const { return {}; }
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
        REGFREG,
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
    std::vector<Reg> def() const {
        return std::visit([](auto&& instr) { return instr.def(); }, instr);
    }
    std::vector<Reg> use() const {
        return std::visit([](auto&& instr) { return instr.use(); }, instr);
    }
    std::vector<FReg> fdef() const {
        return std::visit([](auto&& instr) { return instr.fdef(); }, instr);
    }
    std::vector<FReg> fuse() const {
        return std::visit([](auto&& instr) { return instr.fuse(); }, instr);
    }

    std::variant<
        ImmInstr, RegInstr, RegRegInstr, RegImmInstr, BranchInstr,
        FRegInstr, FRegRegInstr, RegFRegInstr, FRegFRegInstr, FCmpInstr,
        RegImmRegInstr, FRegImmRegInstr, RegLabelInstr,
        JInstr, CallInstr, ReturnInstr
    > instr;
};

typedef Vector<MachineInstr> MachineInstrs;

} // namespace Backend
