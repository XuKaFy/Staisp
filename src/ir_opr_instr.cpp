#include "ir_opr_instr.h"

namespace Ir
{

Symbol BinInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = "
        + (is_float(tr) ? "f" : "" ) 
        + (binType == INSTR_DIV ? (is_signed_imm_type(tr) ? "s" : "u") : "") // SDIV and UDIV
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
    return pInstr(new BinInstr(type, tr, oprd1, oprd2));
}

} // namespace ir
