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
        if (type == RegFRegInstrType::FCVT_W_S)
            return format(type, rd, rs, "rtz"); // round to zero
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

inline std::string stringifyLoad(LSType type) {
    return type == LSType::WORD ? "lw" : type == LSType::DWORD ? "ld" : "flw";
}

inline std::string stringifyStore(LSType type) {
    return type == LSType::WORD ? "sw" : type == LSType::DWORD ? "sd" : "fsw";
}

struct LoadInstr {
    LSType type;
    GReg rd; int imm; Reg rs;

    std::string stringify() const {
        return stringifyLS(stringifyLoad(type), rd, imm, rs);
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
        return stringifyLS(stringifyStore(type), rs2, imm, rs1);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs1, rs2}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {
        if (GReg(rs1) == from) rs1 = std::get<Reg>(to);
        if (rs2 == from) rs2 = to;
    }
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
    std::vector<GReg> uses;
    bool tail = false;

    std::string stringify() const {
        return format(tail ? "tail" : "call", label);
    }

    std::vector<GReg> def() const {
        return {
            Reg::T0,
            Reg::T1,
            Reg::T2,
            Reg::A0,
            Reg::A1,
            Reg::A2,
            Reg::A3,
            Reg::A4,
            Reg::A5,
            Reg::A6,
            Reg::A7,
            Reg::T3,
            Reg::T4,
            Reg::T5,
            Reg::T6,

            FReg::FT0,
            FReg::FT1,
            FReg::FT2,
            FReg::FT3,
            FReg::FT4,
            FReg::FT5,
            FReg::FT6,
            FReg::FT7,
            FReg::FA0,
            FReg::FA1,
            FReg::FA2,
            FReg::FA3,
            FReg::FA4,
            FReg::FA5,
            FReg::FA6,
            FReg::FA7,
            FReg::FT8,
            FReg::FT9,
            FReg::FT10,
            FReg::FT11,
        };
    }
    std::vector<GReg> use() const { return uses; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) {}
};

struct ReturnInstr {
    std::vector<GReg> uses;

    std::string stringify() const {
        return "ret";
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return uses; }

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

struct LoadGlobalInstr {
    LSType type;
    GReg rd; std::string label;

    std::string stringify() const {
        if (type == LSType::FLOAT) return format(stringifyLoad(type), rd, label, Reg::T0);
        return format(stringifyLoad(type), rd, label);
    }

    std::vector<GReg> def() const {
        if (type == LSType::FLOAT) return {rd, Reg::T0};
        return {rd};
    }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) { if (rd == from) rd = to; }
    void replace_use(GReg from, GReg to) {}
};

struct StoreGlobalInstr {
    LSType type;
    GReg rs; std::string label;

    std::string stringify() const {
        return format(stringifyStore(type), rs, label, Reg::T0);
    }

    std::vector<GReg> def() const { return {Reg::T0}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) { if (rs == from) rs = to; }
};

enum class StackObjectType {
    LOCAL, CHILD_ARG, PARENT_ARG
};

inline std::string stringify(StackObjectType type) {
    switch (type) {
        case StackObjectType::LOCAL:
            return "L";
        case StackObjectType::CHILD_ARG:
            return "C";
        case StackObjectType::PARENT_ARG:
            return "P";
    }
}

struct LoadStackAddressInstr {
    Reg rd;
    size_t index;
    StackObjectType type = StackObjectType::LOCAL;

    std::string stringify() const {
        return "LSA " + Backend::stringify(rd) + " at " + Backend::stringify(type) + std::to_string(index);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) { if (GReg(rd) == from) rd = std::get<Reg>(to); }
    void replace_use(GReg from, GReg to) {}
};

struct LoadStackInstr {
    LSType type;
    GReg rd;
    size_t index;
    StackObjectType type1 = StackObjectType::LOCAL;

    std::string stringify() const {
        return "LSA-" + stringifyLoad(type) + " " + Backend::stringify(rd) + " at " + Backend::stringify(type1) + std::to_string(index);
    }

    std::vector<GReg> def() const { return {rd}; }
    std::vector<GReg> use() const { return {}; }

    void replace_def(GReg from, GReg to) { if (rd == from) rd = to; }
    void replace_use(GReg from, GReg to) {}
};

struct StoreStackInstr {
    LSType type;
    GReg rs;
    size_t index;
    StackObjectType type1 = StackObjectType::LOCAL;

    std::string stringify() const {
        return "LSA-" + stringifyStore(type) + Backend::stringify(rs) + " at " + Backend::stringify(type1) + std::to_string(index);
    }

    std::vector<GReg> def() const { return {}; }
    std::vector<GReg> use() const { return {rs}; }

    void replace_def(GReg from, GReg to) {}
    void replace_use(GReg from, GReg to) { if (rs == from) rs = to; }
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
        J,
        CALL,
        RETURN,
        LOAD_ADDRESS,
        LOAD_GLOBAL,
        STORE_GLOBAL,

        LOAD_STACK_ADDRESS,
        LOAD_STACK,
        STORE_STACK
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
    template<typename T>
    const T& as() const {
        return std::get<T>(instr);
    }

    std::variant<
        ImmInstr, RegInstr, RegRegInstr, RegImmInstr, BranchInstr,
        FRegInstr, FRegRegInstr, RegFRegInstr, FRegFRegInstr, FCmpInstr,
        LoadInstr, StoreInstr,
        JInstr, CallInstr, ReturnInstr,
        LoadAddressInstr, LoadGlobalInstr, StoreGlobalInstr,
        LoadStackAddressInstr, LoadStackInstr, StoreStackInstr
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
