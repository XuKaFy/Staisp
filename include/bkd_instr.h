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

inline bool is_virtual(GReg reg) {
    return std::visit([](auto reg) { return Backend::is_virtual(reg); }, reg);
}

struct ImmInstr {
    ImmInstrType type;
    Reg rd; int imm;

    std::string stringify() const {
        return format(type, rd, imm);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {}
};

struct RegInstr {
    RegInstrType type;
    Reg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<Reg>(to); }
};

struct RegRegInstr {
    RegRegInstrType type;
    Reg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<Reg>(to);
        if (GReg(rs2) == from) rs2 = std::get<Reg>(to);
    }
};

struct RegImmInstr {
    RegImmInstrType type;
    Reg rd, rs; int imm;

    std::string stringify() const {
        return format(type, rd, rs, imm);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<Reg>(to); }
};

struct BranchInstr {
    BranchInstrType type;
    Reg rs1, rs2; std::string label;

    std::string stringify() const {
        return format(type, rs1, rs2, label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<Reg>(to);
        if (GReg(rs2) == from) rs2 = std::get<Reg>(to);
    }
};

struct FRegInstr {
    FRegInstrType type;
    FReg rd, rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<FReg>(to); }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<FReg>(to); }
};

struct FRegRegInstr {
    FRegRegInstrType type;
    FReg rd; Reg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<FReg>(to); }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<Reg>(to); }
};

struct RegFRegInstr {
    RegFRegInstrType type;
    Reg rd; FReg rs;

    std::string stringify() const {
        return format(type, rd, rs);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<FReg>(to); }
};

struct FRegFRegInstr {
    FRegFRegInstrType type;
    FReg rd, rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<FReg>(to); }
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<FReg>(to);
        if (GReg(rs2) == from) rs2 = std::get<FReg>(to);
    }
};

struct FCmpInstr {
    FCmpInstrType type;
    Reg rd; FReg rs1, rs2;

    std::string stringify() const {
        return format(type, rd, rs1, rs2);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<FReg>(to);
        if (GReg(rs2) == from) rs2 = std::get<FReg>(to);
    }
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

    void replace_def(GReg from, GReg to) { if (rd == from) rd = to; }
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<Reg>(to); }

};

struct StoreInstr {
    LSType type;
    GReg rs2; int imm; Reg rs1;

    std::string stringify() const {
        return stringifyLS(type == LSType::WORD ? "sw" : type == LSType::DWORD ? "sd" : "fsw", rs2, imm, rs1);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<Reg>(to);
        if (rs2 == from) rs2 = to;
    }
};

struct RegLabelInstr {
    RegLabelInstrType type;
    Reg rs; std::string label;

    std::string stringify() const {
        return format(type, rs, label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) { if (GReg(rs) == from) rs = std::get<Reg>(to); }
};

struct JInstr {
    std::string label;

    std::string stringify() const {
        return format("j", label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {}
};

struct CallInstr {
    std::string label;

    std::string stringify() const {
        return format("call", label);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {}
};

struct ReturnInstr {
    std::string stringify() const {
        return "ret";
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {}
};

struct LoadAddressInstr {
    Reg rd; std::string label;

    std::string stringify() const {
        return format("la", rd, label);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {}
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

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {}
};

template<typename... Ts>
struct overloaded : Ts... {
    explicit overloaded(Ts... ts): Ts(ts)... {}
    using Ts::operator()...;
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
    void replace_def(GReg from, GReg to) {
        return std::visit([&](auto&& instr) { return instr.replace_def(from, to); }, instr);
    }
    void replace_use(GReg from, GReg to) {
        return std::visit([&](auto&& instr) { return instr.replace_use(from, to); }, instr);
    }
    template<typename T>
    T& as() {
        return std::get<T>(instr);
    }

    std::variant<
        ImmInstr, RegInstr, RegRegInstr, RegImmInstr, BranchInstr,
        FRegInstr, FRegRegInstr, RegFRegInstr, FRegFRegInstr, FCmpInstr,
        LoadInstr, StoreInstr, RegLabelInstr,
        JInstr, CallInstr, ReturnInstr,
        LoadAddressInstr, LoadStackAddressInstr
    > instr;

    // aux info
    // ordinal number
    int number{-1};
};

inline bool check_itype_immediate(int32_t value) {
    return value >= -0x800 && value <= 0x7ff;
}


typedef List<MachineInstr> MachineInstrs;

} // namespace Backend
