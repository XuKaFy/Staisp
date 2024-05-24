#include "ir_mem_instr.h"

#include <utility>

namespace Ir {

String AllocInstr::instr_print() const {
    return name() + " = alloca " + to_pointed_type(ty)->type_name();
}

String LoadInstr::instr_print() const {
    return name() + " = load " + ty->type_name() + ", " +
           operand(0)->usee->ty->type_name() + " " + operand(0)->usee->name();
}

String StoreInstr::instr_print() const {
    auto &to = operand(0)->usee;
    auto &val = operand(1)->usee;
    auto real_ty = to_pointed_type(to->ty);
    return "store " + real_ty->type_name() + " " + val->name() + ", " +
           to->ty->type_name() + " " + to->name();
}

pInstr make_alloc_instr(pType tr) { return pInstr(new AllocInstr(std::move(tr))); }

pInstr make_load_instr(const pVal& from) { return pInstr(new LoadInstr(from)); }

pInstr make_store_instr(const pVal& to, const pVal& val) {
    return pInstr(new StoreInstr(to, val));
}

} // namespace Ir
