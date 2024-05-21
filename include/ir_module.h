#pragma once

#include "ir_func_defined.h"
#include "ir_global.h"

namespace Ir {

// 一个程序的集合
// 包括若干已定义的函数和若干已定义的全局变量
struct Module {
    String print_module() const;
    void add_func(pFuncDefined f);
    void add_func_declaration(pFunc f);
    void add_hidden_func(pFunc f);
    void add_global(pGlobal g);

    Vector<pFuncDefined> funsDefined;
    Vector<pGlobal> globs;
    Vector<pFunc> funcHidden;
    Vector<pFunc> funsDeclared;
};

typedef Pointer<Module> pModule;

pModule make_module();

} // namespace Ir
