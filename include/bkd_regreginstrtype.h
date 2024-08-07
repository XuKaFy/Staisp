// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class RegRegInstrType {
    ADD,
    ADDW,
    SUB,
    SUBW,
    MUL,
    MULW,
    DIV,
    DIVW,
    REM,
    REMW,
    SLL,
    SLLW,
    SRL,
    SRLW,
    SRA,
    SRAW,
    AND,
    OR,
    XOR,
    SLT,
};

inline constexpr std::string_view REGREGINSTRTYPE_NAME[] = {
    "add",
    "addw",
    "sub",
    "subw",
    "mul",
    "mulw",
    "div",
    "divw",
    "rem",
    "remw",
    "sll",
    "sllw",
    "srl",
    "srlw",
    "sra",
    "sraw",
    "and",
    "or",
    "xor",
    "slt",
};

inline std::string stringify(RegRegInstrType value) {
    return std::string(REGREGINSTRTYPE_NAME[(size_t) value]);
}

}
