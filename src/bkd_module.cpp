#include "bkd_module.h"

namespace Backend {

String Module::print_module() const
{
    String res;

    res += ".text\n";
    res += ".global main\n";
    res += "\n";

    for (auto &&i : globs) {
        res += i.print();
    }

    for (auto &&i : funcs) {
        res += i.generate_asm();
    }

    return res;
}

} // namespace Backend
