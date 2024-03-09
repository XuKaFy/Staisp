#include "ir_mem_instr.h"

namespace Ir {

Symbol AllocInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = alloca "
        + tr->type_name()
    );
}

Symbol LoadInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = load "
        + tr->type_name()
        + ", "
        + from->tr->type_name()
        + " "
        + from->print()
    );
}

Symbol StoreInstr::instr_print_impl() const
{
    return to_symbol(
        String("store ")
        + tr->type_name()
        + " "
        + val->print()
        + ", "
        + to->tr->type_name()
        + " "
        + to->print()
    );
}

Symbol SymInstr::print_impl() const
{
    if(from == SYM_LOCAL)
        return to_symbol(String("%") + sym);
    return to_symbol(String("@") + sym);
}

pInstr make_alloc_instr(pType tr)
{
    return pInstr(new AllocInstr(tr));
}

pInstr make_load_instr(pInstr from)
{
    return pInstr(new LoadInstr(from));
}

pInstr make_store_instr(pInstr to, pVal val)
{
    return pInstr(new StoreInstr(to, val));
}

pInstr make_sym_instr(TypedSym val, SymFrom from)
{
    val.tr = make_pointer_type(val.tr, false);
    return pInstr(new SymInstr(val, from));
}

} // namespace ir
