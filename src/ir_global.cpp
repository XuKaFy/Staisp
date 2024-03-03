#include "ir_global.h"

namespace Ir {

Symbol Global::print_global() const
{
    String ans = val.sym;
    ans += " = global ";
    ans += gImmName[val.tr];
    ans += " ";
    ans += con.print_impl();
    return to_symbol(ans);
}

Symbol Global::print_impl() const
{
    return to_symbol(String("@") + val.sym);
}

pGlobal make_global(TypedSym val, Const con)
{
    return pGlobal(new Global(val, con));
}

} // ir