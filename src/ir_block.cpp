#include "ir_block.h"

namespace Ir {

Symbol Block::print_block(size_t line) const
{
    String s;
    int lineCount = line;
    for(auto i : instrs) {
        if(i->instrType != INSTR_TYPE_NO_REG)
            i->line = lineCount++;
        if(i->instrType != INSTR_TYPE_LABEL)
            s += "  ";
        s += String(i->instr_print()) + "\n";
    }
    return to_symbol(s);
}

Symbol Block::print_impl() const
{
    return to_symbol("[block]");
}

pInstr Block::add_instr(pInstr instr)
{
    instrs.push_back(instr);
    return instr;
}

pBlock make_block()
{
    return pBlock(new Block());
}

} // namespace ir
