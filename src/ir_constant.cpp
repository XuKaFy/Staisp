#include "ir_constant.h"

namespace Ir {

Symbol Const::print_impl() const
{
    static char buf[128];
    sprintf(buf, "%lld", var);
    return to_symbol(buf);
}

pVal make_constant(Immediate var)
{
    return pVal(new Const(var));
}

} // namespace ir
