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

using GReg = std::variant<Reg, FReg>;

inline std::string stringify(GReg reg) {
    return std::visit([](auto reg) { return Backend::stringify(reg); }, reg);
}

struct ImmInstr {
    ImmInstrType type;
    Reg rd; int imm;

    std::string stringify() const {
        return format(type, rd, imm);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }
};

struct RegInstr {
    RegInstrType type;
    Reg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }
};

struct RegRegInstr {
    RegRegInstrType type;
    Reg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }
};

struct RegImmInstr {
    RegImmInstrType type;
    Reg rd, rs; int imm;

    std::string stringify() const {
        return format(type, rd, rs, imm);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }
};

struct BranchInstr {
    BranchInstrType type;
    Reg rs1, rs2; std::string label;

    std::string stringify() const {
        return format(type, rs1, rs2, label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }
};

struct FRegInstr {
    FRegInstrType type;
    FReg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }
};

struct FRegRegInstr {
    FRegRegInstrType type;
    FReg rd; Reg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }
};

struct RegFRegInstr {
    RegFRegInstrType type;
    Reg rd; FReg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }
};

struct FRegFRegInstr {
    FRegFRegInstrType type;
    FReg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }
};

struct FCmpInstr {
    FCmpInstrType type;
    Reg rd; FReg rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }
};

template<typename T1, typename T2>
std::string stringifyLS(std::string instr, T1 rd, int imm, T2 rs) {
    instr += " ";
    instr += stringify(rd);
    instr += ", ";
    instr += stringify(imm);
    instr += "(";
    instr += stringify(rs);
    instr += ")";
    return instr;
}

enum class LSType {
    WORD, DWORD, FLOAT
};

struct LoadInstr {
    LSType type;
    GReg rd; int imm; Reg rs;

    std::string stringify() const {
        return stringifyLS(type == LSType::WORD ? "lw" : type == LSType::DWORD ? "ld" : "flw", rd, imm, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

};

struct StoreInstr {
    LSType type;
    GReg rs2; int imm; Reg rs1;

    std::string stringify() const {
        return stringifyLS(type == LSType::WORD ? "sw" : type == LSType::DWORD ? "sd" : "fsw", rs2, imm, rs1);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }
};

struct RegLabelInstr {
    RegLabelInstrType type;
    Reg rs; std::string label;

    std::string stringify() const {
        return format(type, rs, label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs}; }
};

struct JInstr {
    std::string label;

    std::string stringify() const {
        return format("j", label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }
};

struct CallInstr {
    std::string label;

    std::string stringify() const {
        return format("call", label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }
};

struct ReturnInstr {
    std::string stringify() const {
        return "ret";
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }
};

struct LoadAddressInstr {
    Reg rd; std::string label;

    std::string stringify() const {
        return format("la", rd, label);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }
};

struct LoadStackAddressInstr {
    Reg rd;
    size_t index;
    bool arg{false};

    std::string stringify() const {
        return "LSA " + Backend::stringify(rd) + " from " + (arg ? "$" : "#") + std::to_string(index);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }
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
        LOAD,
        STORE,
        REGLABEL,
        J,
        CALL,
        RETURN,
        LOAD_ADDRESS,

        LOAD_STACK_ADDRESS
    };

    Type instr_type() const { return (Type) instr.index(); }
    std::string stringify() const {
        return std::visit([](auto&& instr) { return instr.stringify(); }, instr);
    }
    std::vector<GReg> def() const {
        return std::visit([](auto&& instr) { return instr.def(); }, instr);
    }
    std::vector<GReg> use() const {
        return std::visit([](auto&& instr) { return instr.use(); }, instr);
    }

    std::variant<
        ImmInstr, RegInstr, RegRegInstr, RegImmInstr, BranchInstr,
        FRegInstr, FRegRegInstr, RegFRegInstr, FRegFRegInstr, FCmpInstr,
        LoadInstr, StoreInstr, RegLabelInstr,
        JInstr, CallInstr, ReturnInstr,
        LoadAddressInstr, LoadStackAddressInstr
    > instr;
};

typedef List<MachineInstr> MachineInstrs;

} // namespace Backend
