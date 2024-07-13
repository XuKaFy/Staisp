#pragma once

#include "bkd_func.h"
#include "bkd_global.h"

namespace Backend {

struct Module {
    String print_module() const;

    Vector<Func> funcs;
    Vector<Global> globs;
};

} // namespace Backend
