// THIS FILE IS MACHINE-GENERATED
// DO NOT EDIT BY HAND

#pragma once

#include <string>

namespace Backend {

enum class FRegImmRegInstrType {
    FLW,
    FSW,
};

inline constexpr std::string_view FREGIMMREGINSTRTYPE_NAME[] = {
    "flw",
    "fsw",
};

inline std::string stringify(FRegImmRegInstrType value) {
    return std::string(FREGIMMREGINSTRTYPE_NAME[(size_t) value]);
}

}
