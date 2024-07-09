// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class FRegRegInstrType {
    FMV_S_X,
    FCVT_S_W,
};

inline constexpr std::string_view FREGREGINSTRTYPE_NAME[] = {
    "fmv.s.x",
    "fcvt.s.w",
};

inline std::string stringify(FRegRegInstrType value) {
    return std::string(FREGREGINSTRTYPE_NAME[(size_t) value]);
}

}
