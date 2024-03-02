#pragma once

#include "ir_func.h"
#include "ir_func_defined.h"
#include "ir_global.h"

namespace Ir
{

struct Module {
    Symbol print_module() const;
    void add_func(pFuncDefined f);
    void add_global(pGlobal g);

    Vector<pFuncDefined> funsDefined;
    Vector<pGlobal> globs;
};

typedef Pointer<Module> pModule;

pModule make_module();

} // namespace ir
