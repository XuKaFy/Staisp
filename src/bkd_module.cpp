#include "bkd_module.h"

namespace Backend {

String Module::print_module() const
{
    String res;

    for (auto &&i : globs) {
        res += i.print();
    }

    for (auto &&i : funcs) {
        res += i.print();
    }

    return res;
}

} // namespace Backend
