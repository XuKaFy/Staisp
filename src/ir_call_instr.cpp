#include "ir_call_instr.h"

namespace Ir {

String CallInstr::instr_print() const {
    String res;
    if (ty->type_type() != TYPE_VOID_TYPE) {
        res = name() + " = ";
    }
    res += "call " + func_ty->ret_type->type_name() + " @" +
           operand(0)->usee->name() + "("; // %1 = call @fun(i32 1, i64 2)

    my_assert(func_ty->arg_type.size() + 1 == operand_size(),
              "Error: size of arguments unequal to the inputs");

    if (func_ty->arg_type.empty()) {
        goto INSTR_PRINT_IMPL_END;
    }

    res += operand(1)->usee->ty->type_name();
    res += " ";
    res += operand(1)->usee->name();

    for (size_t i = 1; i < func_ty->arg_type.size(); ++i) {
        res += ", ";
        res += operand(i + 1)->usee->ty->type_name();
        res += " ";
        res += operand(i + 1)->usee->name();
    }

INSTR_PRINT_IMPL_END:
    res += ")";
    return res;
}

String RetInstr::instr_print() const {
    if (operand_size() != 0U) {
        return "ret " + operand(0)->usee->ty->type_name() + " " +
               operand(0)->usee->name();
    }
    return "ret void";
}

pInstr make_call_instr(const pFunc &func, const Vector<pVal> &args) {
    return pInstr(new CallInstr(func, args));
}

pInstr make_ret_instr(const pVal &oprd) { return pInstr(new RetInstr(oprd)); }

pInstr make_unreachable_instr() { return pInstr(new UnreachableInstr()); }

} // namespace Ir
