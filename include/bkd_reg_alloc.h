#pragma once

#include <bkd_reg.h>
#include <bkd_freg.h>

namespace Backend {

enum class RegisterUsage {
    CallerSaved,
    CalleeSaved,
    Special,
};

inline constexpr RegisterUsage REG_USAGE[] {
    RegisterUsage::Special,     // zero
    RegisterUsage::Special,     // ra
    RegisterUsage::Special,     // sp
    RegisterUsage::Special,     // gp
    RegisterUsage::Special,     // tp
    RegisterUsage::CallerSaved, // t0
    RegisterUsage::CallerSaved, // t1
    RegisterUsage::CallerSaved, // t2
    RegisterUsage::CalleeSaved, // s0
    RegisterUsage::CalleeSaved, // s1
    RegisterUsage::CallerSaved, // a0
    RegisterUsage::CallerSaved, // a1
    RegisterUsage::CallerSaved, // a2
    RegisterUsage::CallerSaved, // a3
    RegisterUsage::CallerSaved, // a4
    RegisterUsage::CallerSaved, // a5
    RegisterUsage::CallerSaved, // a6
    RegisterUsage::CallerSaved, // a7
    RegisterUsage::CalleeSaved, // s2
    RegisterUsage::CalleeSaved, // s3
    RegisterUsage::CalleeSaved, // s4
    RegisterUsage::CalleeSaved, // s5
    RegisterUsage::CalleeSaved, // s6
    RegisterUsage::CalleeSaved, // s7
    RegisterUsage::CalleeSaved, // s8
    RegisterUsage::CalleeSaved, // s9
    RegisterUsage::CalleeSaved, // s10
    RegisterUsage::CalleeSaved, // s11
    RegisterUsage::CallerSaved, // t3
    RegisterUsage::CallerSaved, // t4
    RegisterUsage::CallerSaved, // t5
    RegisterUsage::CallerSaved, // t6
};

inline constexpr RegisterUsage FREG_USAGE[] {
    RegisterUsage::CallerSaved, // ft0
    RegisterUsage::CallerSaved, // ft1
    RegisterUsage::CallerSaved, // ft2
    RegisterUsage::CallerSaved, // ft3
    RegisterUsage::CallerSaved, // ft4
    RegisterUsage::CallerSaved, // ft5
    RegisterUsage::CallerSaved, // ft6
    RegisterUsage::CallerSaved, // ft7
    RegisterUsage::CalleeSaved, // fs0
    RegisterUsage::CalleeSaved, // fs1
    RegisterUsage::CallerSaved, // fa0
    RegisterUsage::CallerSaved, // fa1
    RegisterUsage::CallerSaved, // fa2
    RegisterUsage::CallerSaved, // fa3
    RegisterUsage::CallerSaved, // fa4
    RegisterUsage::CallerSaved, // fa5
    RegisterUsage::CallerSaved, // fa6
    RegisterUsage::CallerSaved, // fa7
    RegisterUsage::CalleeSaved, // fs2
    RegisterUsage::CalleeSaved, // fs3
    RegisterUsage::CalleeSaved, // fs4
    RegisterUsage::CalleeSaved, // fs5
    RegisterUsage::CalleeSaved, // fs6
    RegisterUsage::CalleeSaved, // fs7
    RegisterUsage::CalleeSaved, // fs8
    RegisterUsage::CalleeSaved, // fs9
    RegisterUsage::CalleeSaved, // fs10
    RegisterUsage::CalleeSaved, // fs11
    RegisterUsage::CallerSaved, // ft8
    RegisterUsage::CallerSaved, // ft9
    RegisterUsage::CallerSaved, // ft10
    RegisterUsage::CallerSaved, // ft11
};

}