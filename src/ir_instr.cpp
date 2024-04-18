#include "ir_instr.h"

namespace Ir {

Symbol Instr::instr_print()
{
    if(instr_str_form == nullptr) {
        return instr_str_form = instr_print_impl();
    }
    return instr_str_form;
}

Symbol Instr::instr_print_impl() const
{
    return name();
}

pInstr make_empty_instr()
{
    static pInstr empty = pInstr();
    return empty;
}

pInstr make_sym_instr(TypedSym sym)
{
    auto j = pInstr(new Instr(sym.ty));
    j->set_name(sym.sym);
    return j;
}

}  // namespace ir

