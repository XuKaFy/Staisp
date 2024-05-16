#include "ir_global.h"

namespace Ir {

Symbol Global::print_global() const
{
    String ans = name();
    ans += " = global ";
    ans += to_pointed_type(ty)->type_name();
    ans += " ";
    ans += con.name();
    return to_symbol(ans);
}

pGlobal make_global(TypedSym val, Const con, bool is_const)
{
    return pGlobal(new Global(val, con, is_const));
}

} // ir
