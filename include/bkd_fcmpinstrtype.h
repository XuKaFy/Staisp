// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class FCmpInstrType {
    FEQ_S,
    FLT_S,
    FLE_S,
};

inline constexpr std::string_view FCMPINSTRTYPE_NAME[] = {
    "feq.s",
    "flt.s",
    "fle.s",
};

inline std::string stringify(FCmpInstrType value) {
    return std::string(FCMPINSTRTYPE_NAME[(size_t) value]);
}

}
