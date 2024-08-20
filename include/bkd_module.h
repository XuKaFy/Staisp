#pragma once

#include <ir_module.h>

#include "bkd_func.h"
#include "bkd_global.h"

namespace Backend {

struct Module {
    String print_module() const;

    Ir::pModule mod;
    Vector<Func> funcs;
    Vector<Global> globs;
};

} // namespace Backend
