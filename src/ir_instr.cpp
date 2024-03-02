#include "ir_instr.h"

namespace Ir {

Symbol Instr::instr_print()
{
    if(instr_str_form == nullptr) {
        return instr_str_form = instr_print_impl();
    }
    return instr_str_form;
}

Symbol Instr::print_impl() const
{
    static char buf[128];
    sprintf(buf, "%%%d", line);
    return to_symbol(String(buf));
}

Symbol BinInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = "
        + gBinInstrName[binType] 
        + " " 
        + gImmName[tr]
        + " "
        + oprd[0]->print()
        + ", "
        + oprd[1]->print());
}

Symbol UnaryInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = "
        + gUnaryInstrName[unaryType]
        + " " 
        + gImmName[tr]
        + " "
        + oprd->print());
}

pInstr make_unary_instr(UnaryInstrType type, ImmType tr, pVal oprd)
{
    return pInstr(new UnaryInstr(type, tr, oprd));
}

pInstr make_binary_instr(BinInstrType type, ImmType tr, pVal oprd1, pVal oprd2)
{
    static pVal tmp[2];
    tmp[0] = oprd1, tmp[1] = oprd2;
    return pInstr(new BinInstr(type, tr, tmp));
}

}  // namespace ir

