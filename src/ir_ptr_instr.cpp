#include "ir_ptr_instr.h"

namespace Ir {

Symbol RefInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = ptrtoint "
        + gImmName[obj->tr]
        + "* "
        + obj->print()
        + " to "
        + gImmName[tr]);
}

Symbol DerefInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = inttoptr "
        + gImmName[IMM_U64]
        + " "
        + obj->print()
        + " to "
        + gImmName[tr]
        + "*");
}

pInstr make_ref_instr(pInstr obj)
{
    return pInstr(new RefInstr(obj));
}

pInstr make_deref_instr(pInstr obj, ImmType tr)
{
    return pInstr(new DerefInstr(obj, tr));
}

} // namespace ir
