// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class RegFRegInstrType {
    FMV_X_S,
    FCVT_W_S,
};

inline constexpr std::string_view REGFREGINSTRTYPE_NAME[] = {
    "fmv.x.s",
    "fcvt.w.s",
};

inline std::string stringify(RegFRegInstrType value) {
    return std::string(REGFREGINSTRTYPE_NAME[(size_t) value]);
}

}
