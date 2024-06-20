#pragma once

#include "bkd_func.h"
#include "bkd_global.h"

namespace Backend {

struct Module {
    Vector<pFunc> funcs;
    Vector<pGlobal> globs;
};

typedef Pointer<Module> pModule;

} // namespace Backend
