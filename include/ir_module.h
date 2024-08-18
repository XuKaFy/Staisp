#pragma once

#include "ir_func_defined.h"
#include "ir_global.h"

namespace Ir {

// 一个程序的集合
// 包括若干已定义的函数和若干已定义的全局变量
struct Module {
    String print_module() const;
    void add_func(const pFuncDefined &f);
    void add_func_declaration(const pFunc &f);
    void add_global(const pGlobal &g);

    void remove_unused_function();
    void remove_unused_global();

    Vector<pFuncDefined> funsDefined;
    Vector<pGlobal> globs;
    Vector<pFunc> funsDeclared;
};

using pModule = Pointer<Module>;

pModule make_module();

} // namespace Ir
