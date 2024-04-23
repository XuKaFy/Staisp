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
    for(size_t i=1; i<operand_size(); ++i) {
        ans += String(", ")
            + operand(i)->usee->ty->type_name()
            + " "
            + operand(i)->usee->name();
    }
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
    :  Instr(ex_shell(val->ty, index.size()))
{
    add_operand(val);
    // set_name("%%");
    // val->set_name("%%%%");
    // printf("%s", val->name());
    for(auto j : index) {
        // printf("[%s]", j->name());
        add_operand(j);
    }
    // puts("");
    // printf("generated: %s\n", instr_print());
    // val->set_name(nullptr);
    // set_name(nullptr);
}

pInstr make_item_instr(pVal val, Vector<pVal> index)
{
    return pInstr(new ItemInstr(val, index));
}

} // namespace ir
