#include "ir_constant.h"

namespace Ir {

Symbol Const::print_impl() const
{
    switch(v.type()) {
    case VALUE_IMM:
        return v.imm_value().print();
    case VALUE_POINTER:
        return "[ptr]";
    case VALUE_ARRAY:
        return "[array]";
    case VALUE_STRUCT:
        return "[struct]";
    }
    throw Exception(1, "Const", "unknown value type");
    return "";
}

pVal make_constant(ImmValue var)
{
    return pVal(new Const(var));
}

} // namespace ir
