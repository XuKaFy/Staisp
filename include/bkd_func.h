#pragma once

#include <utility>

#include "bkd_block.h"
#include "def.h"

namespace Backend {

struct Func {
    std::string name;

    explicit Func(std::string name)
        : name(std::move(name)) {}

    String print() const;

    Vector<Block> body;
};

} // namespace Backend
