// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class RegImmInstrType {
    ADDI,
    ADDIW,
    SLLI,
    SLLIW,
    SRLI,
    SRLIW,
    SRAI,
    SRAIW,
};

inline constexpr std::string_view REGIMMINSTRTYPE_NAME[] = {
    "addi",
    "addiw",
    "slli",
    "slliw",
    "srli",
    "srliw",
    "srai",
    "sraiw",
};

inline std::string stringify(RegImmInstrType value) {
    return std::string(REGIMMINSTRTYPE_NAME[(size_t) value]);
}

}
