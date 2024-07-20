// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class RegLabelInstrType {
    BEQZ,
    BNEZ,
    BLTZ,
    BGEZ,
    BGTZ,
    BLEZ,
};

inline constexpr std::string_view REGLABELINSTRTYPE_NAME[] = {
    "beqz",
    "bnez",
    "bltz",
    "bgez",
    "bgtz",
    "blez",
};

inline std::string stringify(RegLabelInstrType value) {
    return std::string(REGLABELINSTRTYPE_NAME[(size_t) value]);
}

}
