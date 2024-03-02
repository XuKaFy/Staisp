#include "ir_mem_instr.h"

namespace Ir {

Symbol AllocInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = alloca "
        + gImmName[tr]
    );
}

Symbol LoadInstr::instr_print_impl() const
{
    return to_symbol(
        String(print_impl())
        + " = load "
        + gImmName[tr]
        + ", "
        + gImmName[from->tr]
        + "* "
        + from->print()
    );
}

Symbol StoreInstr::instr_print_impl() const
{
    return to_symbol(
        String("store ")
        + gImmName[tr]
        + " "
        + val->print()
        + ", "
        + gImmName[to->tr]
        + "* "
        + to->print()
    );
}

Symbol SymInstr::print_impl() const
{
    if(from == SYM_LOCAL)
        return to_symbol(String("%") + sym->name);
    return to_symbol(String("@") + sym->name);
}

pInstr make_alloc_instr(ImmType tr)
{
    return pInstr(new AllocInstr(tr));
}

pInstr make_load_instr(ImmType tr, pInstr from)
{
    return pInstr(new LoadInstr(tr, from));
}

pInstr make_store_instr(ImmType tr, pInstr to, pVal val)
{
    return pInstr(new StoreInstr(tr, to, val));
}

pInstr make_sym_instr(pSym sym, SymFrom from)
{
    return pInstr(new SymInstr(sym, from));
}

} // namespace ir
