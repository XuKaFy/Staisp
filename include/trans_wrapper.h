#pragma once

#include "def.h"
#include "ir_func_defined.h"
#include "ir_module.h"
namespace Optimize {
class memoi_wrapper {

public:
    Ir::Module *module;
    static const int CACHE_MEMOI_SIZE = 4096;
    static bool is_purely_recursive(Ir::FuncDefined *func);
    memoi_wrapper(Ir::Module *module) : module(module) {};
    memoi_wrapper() = delete;

    void ap();
    static void bk_fill(String &res);
};
} // namespace Optimize