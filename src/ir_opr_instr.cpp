#include "ir_opr_instr.h"

namespace Ir
{

Symbol UnaryInstr::instr_print_impl() const
{
    return to_symbol(
        String(name())
        + " = "
        + gUnaryInstrName[unaryType]
        + " " 
        + ty->type_name()
        + " "
        + operand(0)->usee->name());
}

Symbol BinInstr::instr_print_impl() const
{
    return to_symbol(
        String(name())
        + " = "
        + (is_float(ty) ? "f" : 
            (binType == INSTR_DIV ? (is_signed_type(ty) ? "s" : "u") : "")) 
        + gBinInstrName[binType] 
        + " " 
        + ty->type_name()
        + " "
        + operand(0)->usee->name()
        + ", "
        + operand(1)->usee->name());
}

pInstr make_unary_instr(UnaryInstrType type, pVal oprd)
{
    return pInstr(new UnaryInstr(type, oprd));
}

pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2)
{
    return pInstr(new BinInstr(type, oprd1, oprd2));
}

} // namespace ir
