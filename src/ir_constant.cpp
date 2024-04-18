#include "ir_constant.h"

namespace Ir {

pVal make_constant(Value var)
{
    return pVal(new Const(var));
}

} // namespace ir
