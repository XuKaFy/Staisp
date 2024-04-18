#include "ir_control_instr.h"

namespace Ir {

Symbol LabelInstr::instr_print_impl() const
{
    return to_symbol(name() + String(":"));
}

Symbol BrInstr::instr_print_impl() const
{
    String ans = "br label %";
    ans += operand(0)->usee->name();
    return to_symbol(ans);
}

Symbol BrCondInstr::instr_print_impl() const
{
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
    return to_symbol(ans);
}

pInstr make_label_instr()
{
    return pInstr(new LabelInstr());
}

pInstr make_br_instr(pInstr to)
{
    return pInstr(new BrInstr(to));
}

pInstr make_br_cond_instr(pVal cond, pInstr trueTo, pInstr falseTo)
{
    return pInstr(new BrCondInstr(cond, trueTo, falseTo));
}

} // namespace ir
