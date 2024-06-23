// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class FRegFRegInstrType {
    FADD_S,
    FSUB_S,
    FMUL_S,
    FDIV_S,
    FEQ_S,
    FLT_S,
    FLE_S,
};

inline constexpr std::string_view FREGFREGINSTRTYPE_NAME[] = {
    "fadd.s",
    "fsub.s",
    "fmul.s",
    "fdiv.s",
    "feq.s",
    "flt.s",
    "fle.s",
};

inline std::string stringify(FRegFRegInstrType value) {
    return std::string(FREGFREGINSTRTYPE_NAME[(size_t) value]);
}

}
