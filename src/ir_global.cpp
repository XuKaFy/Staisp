#include "ir_global.h"

#include <memory>

#include <utility>

namespace Ir {

String Global::print_global() const {
    String ans = name();
    ans += " = global ";
    ans += to_pointed_type(ty)->type_name();
    ans += " ";
    ans += con.name();
    return ans;
}

pGlobal make_global(const TypedSym &val, Const con, bool is_const) {
    return std::make_shared<Global>(val, std::move(con), is_const);
}

} // namespace Ir
