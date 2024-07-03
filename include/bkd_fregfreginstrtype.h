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
};

inline constexpr std::string_view FREGFREGINSTRTYPE_NAME[] = {
    "fadd.s",
    "fsub.s",
    "fmul.s",
    "fdiv.s",
};

inline std::string stringify(FRegFRegInstrType value) {
    return std::string(FREGFREGINSTRTYPE_NAME[(size_t) value]);
}

}
