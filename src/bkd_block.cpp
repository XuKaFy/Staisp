#include "bkd_block.h"

namespace Backend {

String Block::print() const
{
    String res = label->instr_print() + "\n";
    
    for (auto &&i : body) {
        res += i->instr_print();
    }

    return res;
}

} // namespace Backend
