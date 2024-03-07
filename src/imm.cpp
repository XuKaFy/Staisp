#include "imm.h"

ImmTypedSym::ImmTypedSym(Symbol sym, ImmType tr, bool is_const)
    : sym(sym), tr(tr), is_const(is_const)
{
}

bool is_signed_imm_type(ImmType tr)
{
    switch(tr) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64:
    case IMM_F32:
    case IMM_F64:
        return true;
    default:
        return false;
    }
}

ImmValue::operator bool() const
{
    switch(ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64: {
        return val.ival;
    }
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64: {
        return val.uval;
    }
    case IMM_F32: {
        return val.f32val;
    }
    case IMM_F64: {
        return val.f64val;
    }
    }
    my_assert(false, "?");
    return ImmValue();
}

ImmValue ImmValue::cast_to(ImmType new_ty) const
{
#define SWITCHES(x) \
        switch(new_ty) { \
        case IMM_I1: \
        case IMM_I8: \
        case IMM_I16: \
        case IMM_I32: \
        case IMM_I64: { \
            return ImmValue((long long)(x), new_ty); \
        } \
        case IMM_U1: \
        case IMM_U8: \
        case IMM_U16: \
        case IMM_U32: \
        case IMM_U64: { \
            return ImmValue((unsigned long long)(x), new_ty); \
        } \
        case IMM_F32: { \
            return ImmValue((float)(x)); \
        } \
        case IMM_F64: { \
            return ImmValue((double)(x)); \
        } \
        }
    switch(ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64: {
        SWITCHES(val.ival)
    }
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64: {
        SWITCHES(val.uval)
    }
    case IMM_F32: {
        SWITCHES(val.f32val)
    }
    case IMM_F64: {
        SWITCHES(val.f64val)
    }
    }
#undef SWITCHES
    my_assert(false, "?");
    return ImmValue();
}

#define OPR_DEF(y) \
ImmValue ImmValue::operator y (ImmValue o) const \
{ \
    auto com_ty = join_imm_type(ty, o.ty); \
    ImmValue a1 = cast_to(com_ty); \
    ImmValue a2 = o.cast_to(com_ty); \
    switch(com_ty) { \
    case IMM_I1: \
    case IMM_I8: \
    case IMM_I16: \
    case IMM_I32: \
    case IMM_I64: { \
        return ImmValue(a1.val.ival y a2.val.ival, com_ty); \
    } \
    case IMM_U1: \
    case IMM_U8: \
    case IMM_U16: \
    case IMM_U32: \
    case IMM_U64: { \
        return ImmValue(a1.val.uval y a2.val.uval, com_ty); \
    } \
    case IMM_F32: { \
        return ImmValue(a1.val.f32val y a2.val.f32val); \
    } \
    case IMM_F64: { \
        return ImmValue(a1.val.f64val y a2.val.f64val); \
    } \
    } \
    return ImmValue(); \
}
OPR_DEF(+)
OPR_DEF(-)
OPR_DEF(*)
OPR_DEF(/)
#undef OPR_DEF

#define OPR_DEF_INT(y) \
ImmValue ImmValue::operator y (ImmValue o) const \
{ \
    auto com_ty = join_imm_type(ty, o.ty); \
    ImmValue a1 = cast_to(com_ty); \
    ImmValue a2 = o.cast_to(com_ty); \
    switch(com_ty) { \
    case IMM_I1: \
    case IMM_I8: \
    case IMM_I16: \
    case IMM_I32: \
    case IMM_I64: { \
        return ImmValue(a1.val.ival y a2.val.ival, com_ty); \
    } \
    case IMM_U1: \
    case IMM_U8: \
    case IMM_U16: \
    case IMM_U32: \
    case IMM_U64: { \
        return ImmValue(a1.val.uval y a2.val.uval, com_ty); \
    } \
    default:\
        my_assert(false, "int to float"); \
    } \
    return ImmValue(); \
}
OPR_DEF_INT(%)
OPR_DEF_INT(&)
OPR_DEF_INT(|)
#undef OPR_DEF_INT

#define OPR_DEF_LOGICAL(y) \
ImmValue ImmValue::operator y (ImmValue o) const \
{ \
    auto com_ty = join_imm_type(ty, o.ty); \
    ImmValue a1 = cast_to(com_ty); \
    ImmValue a2 = o.cast_to(com_ty); \
    switch(com_ty) { \
    case IMM_I1: \
    case IMM_I8: \
    case IMM_I16: \
    case IMM_I32: \
    case IMM_I64: { \
        return ImmValue(a1.val.ival y a2.val.ival); \
    } \
    case IMM_U1: \
    case IMM_U8: \
    case IMM_U16: \
    case IMM_U32: \
    case IMM_U64: { \
        return ImmValue(a1.val.uval y a2.val.uval); \
    } \
    case IMM_F32: { \
        return ImmValue(a1.val.f32val y a2.val.f32val); \
    } \
    case IMM_F64: { \
        return ImmValue(a1.val.f64val y a2.val.f64val); \
    } \
    } \
    return ImmValue(); \
}
OPR_DEF_LOGICAL(&&)
OPR_DEF_LOGICAL(||)
OPR_DEF_LOGICAL(<)
OPR_DEF_LOGICAL(<=)
OPR_DEF_LOGICAL(>)
OPR_DEF_LOGICAL(>=)
OPR_DEF_LOGICAL(==)
OPR_DEF_LOGICAL(!=)
#undef OPR_DEF_INT

ImmType join_imm_type(ImmType a, ImmType b)
{
    return std::max(a, b);
}