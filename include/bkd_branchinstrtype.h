// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class BranchInstrType {
    BEQ,
    BNE,
    BLT,
    BGE,
    BGT,
    BLE,
};

inline constexpr std::string_view BRANCHINSTRTYPE_NAME[] = {
    "beq",
    "bne",
    "blt",
    "bge",
    "bgt",
    "ble",
};

inline std::string stringify(BranchInstrType value) {
    return std::string(BRANCHINSTRTYPE_NAME[(size_t) value]);
}

}
