#include "bkd_func.h"

namespace Backend {

String Func::print() const
{
    String res;

    res += name;
    res += ":\n";

    res += generate_prolog().print();
    for (auto &&block : body) {
        res += block.print();
    }
    res += generate_epilog().print();

    return res;
}

} // namespace Backend
