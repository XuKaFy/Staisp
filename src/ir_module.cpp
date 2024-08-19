#include "ir_module.h"
#include "def.h"

#include <memory>
#include <trans_wrapper.h>

namespace Ir {

void Module::add_func(const pFuncDefined &f) { funsDefined.push_back(f); }

void Module::add_func_declaration(const pFunc &f) { funsDeclared.push_back(f); }

void Module::add_global(const pGlobal &g) { globs.push_back(g); }

String Module::print_module() const {
    String ans;

    for (const auto &i : funsDeclared) {
        ans += i->print_func_declaration();
        ans += "\n";
    }

    for (const auto &i : globs) {
        ans += i->print_global();
        ans += "\n";
    }

    ans += "\n";

    for (const auto &i : funsDefined) {
        ans += i->print_func();
        ans += "\n";
    }

    return ans;
}

void Module::remove_unused_function()
{
    for (auto i = funsDefined.begin(); i != funsDefined.end(); ) {
        if (!funsCache.count(*i) && (*i)->users().empty() && (*i)->name() != "main") {
            i = funsDefined.erase(i);
        } else ++i;
    }
}

void Module::remove_unused_global()
{
    for (auto i = globs.begin(); i != globs.end(); ) {
        if ((*i)->users().empty()) {
            i = globs.erase(i);
        } else ++i;
    }
}

pModule make_module() { return std::make_shared<Module>(); }

} // namespace Ir