// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class RegInstrType {
    MV,
    NEGW,
    SEQZ,
    SNEZ,
    SLTZ,
    SGTZ,
};

inline constexpr std::string_view REGINSTRTYPE_NAME[] = {
    "mv",
    "negw",
    "seqz",
    "snez",
    "sltz",
    "sgtz",
};

inline std::string stringify(RegInstrType value) {
    return std::string(REGINSTRTYPE_NAME[(size_t) value]);
}

}
