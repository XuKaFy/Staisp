#include "ir_constant.h"

namespace Ir {

Const::Const(ImmValue var)
    : ty(make_basic_type(var.ty, false))
{
    var_mem.realloc(8);
    *((unsigned long long*)var_mem.mem.get()) = var.val.uval;
}

Const::Const(pType ty)
    : ty(ty)
{
    size_t len = std::static_pointer_cast<CompoundType>(ty)->length();
    var_mem.realloc(len);
}

Symbol Const::print_impl() const
{
    static char buf[128];
    switch(ty->type_type()) {
    case TYPE_BASIC_TYPE: {
        auto imm_ty = std::static_pointer_cast<BasicType>(ty)->ty;
        ImmValue var(var_mem, imm_ty);
        switch(imm_ty) {
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
        break;
    }
    default:
        sprintf(buf, "[Compound Value %d]", ty->type_type());
    }
    return to_symbol(buf);
}

pVal make_constant(ImmValue var)
{
    return pVal(new Const(var));
}

pVal make_constant(pType ty)
{
    if(ty->type_type() == TYPE_BASIC_TYPE) {
        return make_constant(std::static_pointer_cast<BasicType>(ty)->ty);
    }
    my_assert(false, "not implemented");
    return pVal();
}

} // namespace ir
