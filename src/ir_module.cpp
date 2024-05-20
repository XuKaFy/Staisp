#include "ir_module.h"

namespace Ir {

void Module::add_func(pFuncDefined f) { funsDefined.push_back(f); }

void Module::add_func_declaration(pFunc f) { funsDeclared.push_back(f); }

void Module::add_global(pGlobal g) { globs.push_back(g); }

Symbol Module::print_module() const {
    String ans;

    for (auto i : funsDeclared) {
        ans += i->print_func_declaration();
        ans += "\n";
    }

    for (auto i : globs) {
        ans += i->print_global();
        ans += "\n";
    }

    ans += "\n";

    for (auto i : funsDefined) {
        ans += i->print_func();
        ans += "\n";
    }

    return to_symbol(ans);
}

pModule make_module() { return pModule(new Module()); }

} // namespace Ir