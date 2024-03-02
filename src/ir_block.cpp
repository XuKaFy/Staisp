#include "ir_block.h"

namespace Ir {

Symbol Block::print_impl() const
{
    String s;
    int lineCount = 0;
    for(auto i : instrs) {
        i->line = lineCount++;
        s += String(i->instr_print()) + "\n";
    }
    return to_symbol(s);
}

pInstr Block::add_instr(pInstr instr)
{
    instrs.push_back(instr);
    return instr;
}

} // namespace ir
