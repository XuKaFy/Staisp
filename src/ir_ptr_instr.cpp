#include "ir_ptr_instr.h"

#include "type.h"

namespace Ir {

#ifdef USING_MINI_GEP
// i32* -> i32*
// [2 x i32]* -> i32*
// [2 x [2 x i32]]* -> [2 x i32]*

pType ex_shell_1(pType ty)
{
    if (!is_array(to_pointed_type(ty))) {
        return ty; // i32* -> i32*
    }
    return make_pointer_type(to_elem_type(to_pointed_type(ty)));
}

MiniGepInstr::MiniGepInstr(const pVal &val, const pVal &index)
    : Instr(ex_shell_1(val->ty))
{
    add_operand(val);
    add_operand(index);
}

String MiniGepInstr::instr_print() const {
    String ans = name() + " = getelementptr " +
                 to_pointed_type(operand(0)->usee->ty)->type_name() + ", " +
                 operand(0)->usee->ty->type_name() + " " +
                 operand(0)->usee->name();

    ans += String(", ") + operand(1)->usee->ty->type_name() + " " +
        operand(1)->usee->name();
    if (is_array(to_pointed_type(operand(0)->usee->ty)))
        ans += ", i32 0"; // 到下一层寻址
    return ans;
}

pInstr make_mini_gep_instr(const pVal &val, const pVal &index)
{
    return pInstr(new MiniGepInstr(val, index));
}
#endif

String ItemInstr::instr_print() const {
    String ans = name() + " = getelementptr " +
                 to_pointed_type(operand(0)->usee->ty)->type_name() + ", " +
                 operand(0)->usee->ty->type_name() + " " +
                 operand(0)->usee->name();

    if (get_from_local) {
        ans += ", i32 0";
    }
    for (size_t i = 1; i < operand_size(); ++i) {
        ans += String(", ") + operand(i)->usee->ty->type_name() + " " +
               operand(i)->usee->name();
    }
    return ans;
}

pType ex_shell(pType t, size_t count, bool &get_from_local) {
    /*
    1. int [10][10] -> [2 x [2 x i32]]* : get_from_local
    2. int [][10]   -> [2 x i32]*       : !get_from_local
    */
    t = to_pointed_type(t);
    get_from_local = true;
    for (size_t i = 0; i < count; ++i) {
        if (!is_array(t)) {
            if (i != count - 1) {
                throw Exception(1, "ex_shell", "not a right dimension");
            }
            get_from_local = false;
            break;
        }
        t = to_elem_type(t);
    }
    return make_pointer_type(t);
}

ItemInstr::ItemInstr(const pVal &val, const Vector<pVal> &index)
    : Instr(ex_shell(val->ty, index.size(), get_from_local)) {
    add_operand(val);
    // set_name("%%");
    // val->set_name("%%%%");
    // printf("%s", val->name());
    for (const auto &j : index) {
        // printf("[%s]", j->name());
        add_operand(j);
    }
    // puts("");
    // printf("generated: %s\n", instr_print());
    // val->set_name(nullptr);
    // set_name(nullptr);
}

pInstr make_item_instr(const pVal &val, const Vector<pVal> &index) {
    return pInstr(new ItemInstr(val, index));
}

} // namespace Ir
