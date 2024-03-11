#include "ir_ptr_instr.h"

namespace Ir {

/*
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
*/

Symbol ItemInstr::instr_print_impl() const
{
    String ans = 
        String(print_impl())
        + " = getelementptr "
        + to_pointed_type(val->tr)->type_name()
        + ", "
        + val->tr->type_name()
        + " "
        + val->print()
        + ", i32 0";
    for(auto i : index)
        ans += String(", ")
            + i->tr->type_name()
            + " "
            + i->print();
    return to_symbol(ans);
}

/*
pInstr make_ref_instr(pInstr obj)
{
    return pInstr(new RefInstr(obj));
}

pInstr make_deref_instr(pInstr obj)
{
    return pInstr(new DerefInstr(obj));
}
*/

pType ex_shell(pType t, size_t count)
{
    t = to_pointed_type(t);
    for(size_t i=0; i<count; ++i) {
        t = to_elem_type(t);
    }
    return make_pointer_type(t, false);
}

ItemInstr::ItemInstr(pInstr val, Vector<pInstr> index)
    :  Instr(INSTR_TYPE_NEED_REG, ex_shell(val->tr, index.size())), val(val), index(index)
{
    ;
}

pInstr make_item_instr(pInstr val, Vector<pInstr> index)
{
    return pInstr(new ItemInstr(val, index));
}

} // namespace ir
