// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class FRegRegInstrType {
    FCVT_S_W,
    FCVT_W_S,
    FMV_X_W,
    FMV_W_X,
};

inline constexpr std::string_view FREGREGINSTRTYPE_NAME[] = {
    "fcvt.s.w",
    "fcvt.w.s",
    "fmv.x.w",
    "fmv.w.x",
};

inline std::string stringify(FRegRegInstrType value) {
    return std::string(FREGREGINSTRTYPE_NAME[(size_t) value]);
}

}
