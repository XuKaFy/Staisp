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

}  // namespace ir

