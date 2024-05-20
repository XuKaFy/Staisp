#include "ir_global.h"

namespace Ir {

String Global::print_global() const {
    String ans = name();
    ans += " = global ";
    ans += to_pointed_type(ty)->type_name();
    ans += " ";
    ans += con.name();
    return ans;
}

pGlobal make_global(TypedSym val, Const con, bool is_const) {
    return pGlobal(new Global(val, con, is_const));
}

} // namespace Ir
