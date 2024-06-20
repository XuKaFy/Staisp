#include "bkd_func.h"

namespace Backend {

String Func::print() const
{
    String res;
    
    for (auto &&i : body) {
        res += i.print();
    }

    return res;
}

} // namespace Backend
