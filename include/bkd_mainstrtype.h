// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string_view>

namespace Backend {

enum class MAInstrType {
    FMADD_S,
    FMSUB_S,
    FNMADD_S,
    FNMSUB_S,
};

inline constexpr std::string_view MAINSTRTYPE_NAME[] = {
    "fmadd.s",
    "fmsub.s",
    "fnmadd.s",
    "fnmsub.s",
};

inline std::string stringify(MAInstrType value) {
    return std::string(MAINSTRTYPE_NAME[(size_t) value]);
}

}
