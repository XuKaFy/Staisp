#include "ir_ptr_instr.h"

namespace Ir {

Symbol RefInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = ptrtoint "
        + obj->tr->type_name()
        + "* "
        + obj->print()
        + " to "
        + tr->type_name());
}

Symbol DerefInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = inttoptr "
        + gImmName[ARCH_USED_POINTER_TYPE]
        + " "
        + obj->print()
        + " to "
        + tr->type_name()
        + "*");
}

Symbol ItemInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = getelementptr "
        + to_pointed_type(val->tr)->type_name()
        + ", "
        + val->tr->type_name()
        + " "
        + val->print()
        + ", i32 0, "
        + index->tr->type_name()
        + " "
        + index->print());
}

pInstr make_ref_instr(pInstr obj)
{
    return pInstr(new RefInstr(obj));
}

pInstr make_deref_instr(pInstr obj)
{
    return pInstr(new DerefInstr(obj));
}

pInstr make_item_instr(pInstr val, pInstr index)
{
    return pInstr(new ItemInstr(val, index));
}

} // namespace ir
