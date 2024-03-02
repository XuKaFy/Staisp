#include "ir_block.h"

namespace Ir {

Symbol Block::print_impl() const
{
    String s;
    for(auto i : instrs) {
        s += String(i->print()) + "\n";
    }
    return to_symbol(s);
}

void Block::add_instr(pInstr instr)
{
    instrs.push_back(instr);
}

} // namespace ir
