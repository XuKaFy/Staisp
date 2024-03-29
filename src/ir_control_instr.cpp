#include "ir_control_instr.h"

namespace Ir {

Symbol LabelInstr::instr_print_impl() const
{
    static char buf[128];
    sprintf(buf, "%d:", line);
    return to_symbol(String(buf));
}

Symbol BrInstr::instr_print_impl() const
{
    String ans = "br label ";
    ans += to->print_impl();
    return to_symbol(ans);
}

Symbol BrCondInstr::instr_print_impl() const
{
    String ans = "br ";
    ans += cond->tr->type_name();
    ans += " ";
    ans += cond->print();
    ans += ", label ";
    ans += trueTo->print_impl();
    ans += ", label ";
    ans += falseTo->print_impl();
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

pInstr make_br_cond_instr(pInstr cond, pInstr trueTo, pInstr falseTo)
{
    return pInstr(new BrCondInstr(cond, trueTo, falseTo));
}

} // namespace ir
