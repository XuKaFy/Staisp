#include "ir_cmp_instr.h"

namespace Ir {

Symbol CmpInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + (is_float(tr) ? " = fcmp " : " = icmp ")
        + gCmpInstrName[cmp_type]
        + " " 
        + ch[0]->tr->type_name()
        + " "
        + ch[0]->print()
        + ", "
        + ch[1]->print());
}

pInstr make_cmp_instr(CmpType cmp_type, pType tr, pInstr a1, pInstr a2)
{
    return pInstr(new CmpInstr(cmp_type, tr, a1, a2 ));
}

} // namespace ir
