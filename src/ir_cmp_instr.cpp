#include "ir_cmp_instr.h"

namespace Ir {

Symbol CmpInstr::instr_print_impl() const
{
    return to_symbol(
        String(name())
        + (is_float(operand(0)->usee->ty) ? " = fcmp " : " = icmp ")
        + gCmpInstrName[cmp_type]
        + " " 
        + operand(0)->usee->ty->type_name()
        + " "
        + operand(0)->usee->name()
        + ", "
        + operand(1)->usee->name());
}

pInstr make_cmp_instr(CmpType cmp_type, pVal a1, pVal a2)
{
    return pInstr(new CmpInstr(cmp_type, a1, a2));
}

} // namespace ir
