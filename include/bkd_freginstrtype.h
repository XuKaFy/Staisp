// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class FRegInstrType {
    FMV_S,
    FNEG_S,
};

inline constexpr std::string_view FREGINSTRTYPE_NAME[] = {
    "fmv.s",
    "fneg.s",
};

inline std::string stringify(FRegInstrType value) {
    return std::string(FREGINSTRTYPE_NAME[(size_t) value]);
}

}
