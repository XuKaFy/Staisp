#include "bkd_block.h"

namespace Backend {

String Block::print() const
{
    String res = name + ":\n";
    
    for (auto &&i : body) {
        res += "    ";
        res += i.stringify();
        res += "\n";
    }

    return res;
}

} // namespace Backend
