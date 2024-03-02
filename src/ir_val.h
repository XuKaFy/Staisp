#pragma once

#include "def.h"

namespace Ir {

struct Val {
    Symbol print();
    virtual Symbol print_impl() const = 0;

    Symbol str_form { nullptr };
};

typedef Pointer<Val> pVal;

} // namespace ir