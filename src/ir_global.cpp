#include "ir_global.h"

namespace Ir {

Symbol Global::print_global() const
{
    String ans = name->print();
    ans += " = global ";
    ans += gImmName[tr];
    ans += " ";
    ans += con.print_impl();
    return to_symbol(ans);
}

Symbol Global::print_impl() const
{
    return name->print();
}

pGlobal make_global(ImmType tr, Const con, pSym name)
{
    return pGlobal(new Global(tr, con, name));
}

} // ir
