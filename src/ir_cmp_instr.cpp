#include "ir_cmp_instr.h"

namespace Ir {

Symbol CmpInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = icmp "
        + gCmpInstrName[cmp_type]
        + " " 
        + gImmName[ch[0]->tr]
        + " "
        + ch[0]->print()
        + ", "
        + ch[1]->print());
}

pInstr make_cmp_instr(CmpType cmp_type, ImmType tr, pInstr a1, pInstr a2)
{
    return pInstr(new CmpInstr(cmp_type, tr, a1, a2 ));
}

} // namespace ir