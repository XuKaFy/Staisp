#include "ir_control_instr.h"

#include <utility>

namespace Ir {

String LabelInstr::instr_print() const { return name() + ":"; }

String BrInstr::instr_print() const {
    String ans = "br label %";
    ans += operand(0)->usee->name();
    return ans;
}

String BrCondInstr::instr_print() const {
    String ans = "br ";
    auto &cond = operand(0)->usee;
    auto &trueTo = operand(1)->usee;
    auto &falseTo = operand(2)->usee;
    ans += cond->ty->type_name();
    ans += " ";
    ans += cond->name();
    ans += ", label %";
    ans += trueTo->name();
    ans += ", label %";
    ans += falseTo->name();
    return ans;
}

pInstr BrCondInstr::select(bool cond) {
    return make_br_instr(static_cast<Instr *>(operand(static_cast<int>(cond) + 1)->usee));
}

pInstr make_label_instr() { return pInstr(new LabelInstr()); }

pInstr make_br_instr(const pInstr& to) { return pInstr(new BrInstr(to)); }

pInstr make_br_instr(Instr *to) { return pInstr(new BrInstr(to)); }

pInstr make_br_cond_instr(const pVal& cond, const pInstr& trueTo, const pInstr& falseTo) {
    return pInstr(new BrCondInstr(cond, trueTo, falseTo));
}

} // namespace Ir
