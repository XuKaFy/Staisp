#include "ir_ptr_instr.h"

namespace Ir {

Symbol ItemInstr::instr_print_impl() const
{
    String ans = 
        String(name())
        + " = getelementptr "
        + to_pointed_type(operand(0)->usee->ty)->type_name()
        + ", "
        + operand(0)->usee->ty->type_name()
        + " "
        + operand(0)->usee->name()
        + ", i32 0";
    for(auto i : index)
        ans += String(", ")
            + i->ty->type_name()
            + " "
            + i->name();
    return to_symbol(ans);
}

pType ex_shell(pType t, size_t count)
{
    t = to_pointed_type(t);
    for(size_t i=0; i<count; ++i) {
        t = to_elem_type(t);
    }
    return make_pointer_type(t);
}

ItemInstr::ItemInstr(pVal val, Vector<pVal> index)
    :  Instr(ex_shell(val->ty, index.size())), index(index)
{
    add_operand(val);
}

pInstr make_item_instr(pVal val, Vector<pVal> index)
{
    return pInstr(new ItemInstr(val, index));
}

} // namespace ir
