// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class Reg {
    ZERO,
    RA,
    SP,
    GP,
    TP,
    T0,
    T1,
    T2,
    S0,
    FP,
    S1,
    A0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    S8,
    S9,
    S10,
    S11,
    T3,
    T4,
    T5,
    T6,
};

inline constexpr std::string_view REG_NAME[] = {
    "zero",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0",
    "t1",
    "t2",
    "s0",
    "fp",
    "s1",
    "a0",
    "a1",
    "a2",
    "a3",
    "a4",
    "a5",
    "a6",
    "a7",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "s8",
    "s9",
    "s10",
    "s11",
    "t3",
    "t4",
    "t5",
    "t6",
};

inline bool is_virtual(Reg value) {
    return (int) value < 0 || (int) value >= 32;
}

inline std::string stringify(Reg value) {
    if (is_virtual(value)) {
        return "%" + std::to_string(~(int) value);
    }
    return std::string(REG_NAME[(size_t) value]);
}

}
