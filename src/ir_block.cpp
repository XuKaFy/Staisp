#include "ir_block.h"

namespace Ir {

void Block::generate_line(size_t &line) const
{
    String s;
    for(auto i : instrs) {
        if(i->instrType != INSTR_TYPE_NO_REG)
            i->line = line++;
    }
}

Symbol Block::print_block() const
{
    String s;
    for(auto i : instrs) {
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

pInstr Block::label() const
{
    return instrs.front();
}

pInstr Block::add_instr(pInstr instr)
{
    instrs.push_back(instr);
    return instr;
}

pInstr Block::finish_block_with_jump(pBlock b)
{
    return add_instr(make_br_instr(b->label()));
}

pBlock make_block()
{
    return pBlock(new Block());
}

} // namespace ir
