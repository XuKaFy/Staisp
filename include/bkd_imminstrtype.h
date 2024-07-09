// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class ImmInstrType {
    LUI,
    LI,
};

inline constexpr std::string_view IMMINSTRTYPE_NAME[] = {
    "lui",
    "li",
};

inline std::string stringify(ImmInstrType value) {
    return std::string(IMMINSTRTYPE_NAME[(size_t) value]);
}

}
