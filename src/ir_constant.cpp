#include "ir_constant.h"

namespace Ir {

Symbol Const::print_impl() const
{
    static char buf[128];
    switch(var.ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64:
        sprintf(buf, "%lld", var.val.ival);
        break;
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64:
        sprintf(buf, "%llu", var.val.uval);
        break;
    case IMM_F32:
        sprintf(buf, "%f", var.val.f32val);
        break;
    case IMM_F64:
        sprintf(buf, "%lf", var.val.f64val);
        break;
    }
    return to_symbol(buf);
}

pVal make_constant(ImmValue var)
{
    return pVal(new Const(var));
}

} // namespace ir
