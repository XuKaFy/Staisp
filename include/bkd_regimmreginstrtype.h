// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class RegImmRegInstrType {
    LW,
    LD,
    SW,
    SD,
};

inline constexpr std::string_view REGIMMREGINSTRTYPE_NAME[] = {
    "lw",
    "ld",
    "sw",
    "sd",
};

inline std::string stringify(RegImmRegInstrType value) {
    return std::string(REGIMMREGINSTRTYPE_NAME[(size_t) value]);
}

}
