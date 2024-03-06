#include "ir_instr.h"

namespace Ir {

Symbol Instr::instr_print()
{
    if(instr_str_form == nullptr) {
        return instr_str_form = instr_print_impl();
    }
    return instr_str_form;
}

Symbol Instr::print_impl() const
{
    static char buf[128];
    sprintf(buf, "%%%d", line);
    return to_symbol(String(buf));
}

Symbol Instr::instr_print_impl() const
{
    return print_impl();
}

pInstr make_empty_instr()
{
    static pInstr empty = pInstr();
    return empty;
}

pInstr make_const_arg(TypedSym sym)
{
    return pInstr(new ConstArgInstr(sym));
}

ConstArgInstr::ConstArgInstr(TypedSym sym)
    : Instr(INSTR_TYPE_HIDE, sym.tr), sym(sym.sym)
{
}

Symbol ConstArgInstr::instr_print_impl() const
{
    return "";
}

Symbol ConstArgInstr::print_impl() const
{
    return to_symbol(String("%") + sym);
}

bool ConstArgInstr::is_end_of_block() const
{
    return false;
}

}  // namespace ir

