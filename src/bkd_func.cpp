#include "bkd_func.h"

namespace Backend {

String Func::generate_asm() const
{
    String res;

    res += name;
    res += ":\n";

    for (auto &&block : blocks) {
        res += block.print();
    }

    return res;
}

} // namespace Backend
