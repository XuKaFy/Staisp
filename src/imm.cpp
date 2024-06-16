#include "imm.h"

#include <utility>

bool is_imm_signed(ImmType tr) {
    switch (tr) {
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

bool is_imm_float(ImmType tr) {
    switch (tr) {
    case IMM_F32:
    case IMM_F64:
        return true;
    default:
        return false;
    }
}

size_t bytes_of_imm_type(ImmType tr) {
    switch (tr) {
    case IMM_I1:
    case IMM_U1:
        return 1;
    case IMM_I8:
    case IMM_U8:
        return 1;
    case IMM_I16:
    case IMM_U16:
        return 2;
    case IMM_I32:
    case IMM_U32:
    case IMM_F32:
        return 4;
    case IMM_I64:
    case IMM_U64:
    case IMM_F64:
        return 8;
    }
    return 0;
}

bool is_imm_integer(ImmType t) {
    switch (t) {
    case IMM_I1:
    case IMM_U1:
    case IMM_I8:
    case IMM_U8:
    case IMM_I16:
    case IMM_U16:
    case IMM_I32:
    case IMM_U32:
    case IMM_I64:
    case IMM_U64:
        return true;
    case IMM_F32:
    case IMM_F64:
        return false;
    }
    return false;
}

// Yaossg's NOTE:
// proper name: common type
// surprising implementation (and very absurd too)
ImmType join_imm_type(ImmType a, ImmType b) { return std::max(a, b); }

ImmValue::operator bool() const {
    switch (ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64: {
        return val.ival != 0;
    }
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64: {
        return val.uval != 0U;
    }
    case IMM_F32: {
        return val.f32val != 0.0F;
    }
    case IMM_F64: {
        return val.f64val != 0.0;
    }
    }
    throw Exception(1, "ImmValue", "type not implemented {operator bool()}");
}

ImmValue ImmValue::cast_to(ImmType new_ty) const {
#define SWITCHES(x)                                                            \
    switch (new_ty) {                                                          \
    case IMM_I1:                                                               \
    case IMM_I8:                                                               \
    case IMM_I16:                                                              \
    case IMM_I32:                                                              \
    case IMM_I64: {                                                            \
        return ImmValue((long long)(x), new_ty);                               \
    }                                                                          \
    case IMM_U1:                                                               \
    case IMM_U8:                                                               \
    case IMM_U16:                                                              \
    case IMM_U32:                                                              \
    case IMM_U64: {                                                            \
        return ImmValue((unsigned long long)(x), new_ty);                      \
    }                                                                          \
    case IMM_F32: {                                                            \
        return ImmValue((float)(x));                                           \
    }                                                                          \
    case IMM_F64: {                                                            \
        return ImmValue((double)(x));                                          \
    }                                                                          \
    }
    switch (ty) {
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
    printf("Warning: cast from %s to %s", gImmName[ty].c_str(),
           gImmName[new_ty].c_str());
    throw Exception(1, "ImmValue", "type not implemented {cast_to}");
    return {};
}

#define OPR_DEF(y)                                                             \
    ImmValue ImmValue::operator y(ImmValue o) const {                          \
        auto com_ty = join_imm_type(ty, o.ty);                                 \
        ImmValue a1 = cast_to(com_ty);                                         \
        ImmValue a2 = o.cast_to(com_ty);                                       \
        switch (com_ty) {                                                      \
        case IMM_I1:                                                           \
        case IMM_I8:                                                           \
        case IMM_I16:                                                          \
        case IMM_I32:                                                          \
        case IMM_I64: {                                                        \
            return ImmValue(a1.val.ival y a2.val.ival, com_ty);                \
        }                                                                      \
        case IMM_U1:                                                           \
        case IMM_U8:                                                           \
        case IMM_U16:                                                          \
        case IMM_U32:                                                          \
        case IMM_U64: {                                                        \
            return ImmValue(a1.val.uval y a2.val.uval, com_ty);                \
        }                                                                      \
        case IMM_F32: {                                                        \
            return ImmValue(a1.val.f32val y a2.val.f32val);                    \
        }                                                                      \
        case IMM_F64: {                                                        \
            return ImmValue(a1.val.f64val y a2.val.f64val);                    \
        }                                                                      \
        }                                                                      \
        throw Exception(1, "ImmValue", "type not implemented {OPR_DEF}");      \
        return ImmValue();                                                     \
    }
OPR_DEF(+)
OPR_DEF(-)
OPR_DEF(*)
OPR_DEF(/)
#undef OPR_DEF

#define OPR_DEF_INT(y)                                                         \
    ImmValue ImmValue::operator y(ImmValue o) const {                          \
        auto com_ty = join_imm_type(ty, o.ty);                                 \
        ImmValue a1 = cast_to(com_ty);                                         \
        ImmValue a2 = o.cast_to(com_ty);                                       \
        switch (com_ty) {                                                      \
        case IMM_I1:                                                           \
        case IMM_I8:                                                           \
        case IMM_I16:                                                          \
        case IMM_I32:                                                          \
        case IMM_I64: {                                                        \
            return ImmValue(a1.val.ival y a2.val.ival, com_ty);                \
        }                                                                      \
        case IMM_U1:                                                           \
        case IMM_U8:                                                           \
        case IMM_U16:                                                          \
        case IMM_U32:                                                          \
        case IMM_U64: {                                                        \
            return ImmValue(a1.val.uval y a2.val.uval, com_ty);                \
        }                                                                      \
        default:                                                               \
            throw Exception(2, "ImmValue",                                     \
                            "non-integer type does operation on integer");     \
        }                                                                      \
        throw Exception(1, "ImmValue", "type not implemented {OPR_DEF_INT}");  \
        return ImmValue();                                                     \
    }
OPR_DEF_INT(%)
OPR_DEF_INT(&)
OPR_DEF_INT(|)
OPR_DEF_INT(^)
OPR_DEF_INT(>>)
OPR_DEF_INT(<<)
#undef OPR_DEF_INT

#define OPR_DEF_LOGICAL(y)                                                     \
    bool ImmValue::operator y(ImmValue o) const {                          \
        auto com_ty = join_imm_type(ty, o.ty);                                 \
        ImmValue a1 = cast_to(com_ty);                                         \
        ImmValue a2 = o.cast_to(com_ty);                                       \
        switch (com_ty) {                                                      \
        case IMM_I1:                                                           \
        case IMM_I8:                                                           \
        case IMM_I16:                                                          \
        case IMM_I32:                                                          \
        case IMM_I64: {                                                        \
            return a1.val.ival y a2.val.ival;                        \
        }                                                                      \
        case IMM_U1:                                                           \
        case IMM_U8:                                                           \
        case IMM_U16:                                                          \
        case IMM_U32:                                                          \
        case IMM_U64: {                                                        \
            return a1.val.uval y a2.val.uval;                        \
        }                                                                      \
        case IMM_F32: {                                                        \
            return a1.val.f32val y a2.val.f32val;                    \
        }                                                                      \
        case IMM_F64: {                                                        \
            return a1.val.f64val y a2.val.f64val;                    \
        }                                                                      \
        }                                                                      \
        throw Exception(1, "ImmValue",                                         \
                        "type not implemented {OPR_DEF_LOGICAL}");             \
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

String ImmValue::print() const {
    switch (ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64:
        return std::to_string(val.ival);
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64:
        return std::to_string(val.uval);
    case IMM_F32:
    case IMM_F64: {
        char buf[20];
        union {
            double d;
            unsigned long long u;
        };
        u = 0;
        d = ty == IMM_F32 ? val.f32val : val.f64val;
        sprintf(buf, "0x%016llx", u);
        return buf;
    }
    }
    return "[NOT A NUMBER]";
}

ImmValue ImmValue::operator-() const {
    switch (ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64:
        return -val.ival;
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64:
        return -val.uval;
    case IMM_F32:
        return -val.f32val;
    case IMM_F64:
        return -val.f64val;
    }
    return 0;
}

bool ImmValue::operator!() const {
    switch (ty) {
    case IMM_I1:
    case IMM_I8:
    case IMM_I16:
    case IMM_I32:
    case IMM_I64:
        return val.ival == 0;
    case IMM_U1:
    case IMM_U8:
    case IMM_U16:
    case IMM_U32:
    case IMM_U64:
        return val.uval == 0U;
    case IMM_F32:
        return val.f32val == 0.0F;
    case IMM_F64:
        return val.f64val == 0.0;
    }
    throw Exception(1, "ImmValue::operator !()", "?");
    return 0;
}
