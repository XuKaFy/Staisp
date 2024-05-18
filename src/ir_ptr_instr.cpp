#include "ir_ptr_instr.h"
#include "type.h"

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
        + operand(0)->usee->name();
    
    if(get_from_local) {
        ans += ", i32 0";
    }
    for(size_t i=1; i<operand_size(); ++i) {
        ans += String(", ")
            + operand(i)->usee->ty->type_name()
            + " "
            + operand(i)->usee->name();
    }
    return to_symbol(ans);
}

pType ex_shell(pType t, size_t count, bool &get_from_local)
{
    /*
    1. int [10][10] -> [2 x [2 x i32]]* : get_from_local
    2. int [][10]   -> [2 x i32]*       : !get_from_local
    */
    t = to_pointed_type(t);
    get_from_local = true;
    for(size_t i=0; i<count; ++i) {
        if(!is_array(t)) {
            if(i != count - 1) {
                throw Exception(1, "ex_shell", "not a right dimension");
            }
            get_from_local = false;
            break;
        }
        t = to_elem_type(t);
    }
    return make_pointer_type(t);
}

ItemInstr::ItemInstr(pVal val, Vector<pVal> index)
    :  Instr(ex_shell(val->ty, index.size(), get_from_local))
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
