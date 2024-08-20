#pragma once

#include "def.h"
#include "ir_func_defined.h"
#include "ir_module.h"
namespace Optimize {
class memoi_wrapper {

public:
    Ir::Module *module;
    static bool is_purely_recursive(Ir::FuncDefined *func);
    memoi_wrapper(Ir::Module *module) : module(module) {};
    memoi_wrapper() = delete;

    void ap();

    static void bk_fill(Ir::Module* mod, String &res);
};
} // namespace Optimize