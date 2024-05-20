#include "ir_mem_instr.h"

namespace Ir {

Symbol AllocInstr::instr_print_impl() const {
    return to_symbol(String(name()) + " = alloca " +
                     to_pointed_type(ty)->type_name());
}

Symbol LoadInstr::instr_print_impl() const {
    return to_symbol(String(name()) + " = load " + ty->type_name() + ", " +
                     operand(0)->usee->ty->type_name() + " " +
                     operand(0)->usee->name());
}

Symbol StoreInstr::instr_print_impl() const {
    auto &to = operand(0)->usee;
    auto &val = operand(1)->usee;
    auto real_ty = to_pointed_type(to->ty);
    return to_symbol(String("store ") + real_ty->type_name() + " " +
                     val->name() + ", " + to->ty->type_name() + " " +
                     to->name());
}

pInstr make_alloc_instr(pType tr) { return pInstr(new AllocInstr(tr)); }

pInstr make_load_instr(pVal from) { return pInstr(new LoadInstr(from)); }

pInstr make_store_instr(pVal to, pVal val) {
    return pInstr(new StoreInstr(to, val));
}

} // namespace Ir
