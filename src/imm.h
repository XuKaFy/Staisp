#pragma once

#include "def.h"

#define IMM_TYPE_TABLE \
    ENTRY(0, I1, i1) \
    ENTRY(1, U1, u1) \
    ENTRY(2, I8, i8) \
    ENTRY(3, U8, u8) \
    ENTRY(4, I16, i16) \
    ENTRY(5, U16, u16) \
    ENTRY(6, I32, i32) \
    ENTRY(7, U32, u32) \
    ENTRY(8, F32, f32) \
    ENTRY(9, I64, i64) \
    ENTRY(10, U64, u64) \
    ENTRY(11, F64, f64)

enum ImmType {
#define ENTRY(x, y, z) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

bool is_signed_imm_type(ImmType tr);

struct ImmValueOnly {
    ImmValueOnly()
        : ival(0) { }
    ImmValueOnly(long long val)
        : ival(val) { }
    ImmValueOnly(unsigned long long val)
        : uval(val) { }
    ImmValueOnly(float val)
        : f32val(val) { }
    ImmValueOnly(double val)
        : f64val(val) { }
    union {
        long long ival;
        unsigned long long uval;
        float f32val;
        double f64val;
    };
};

struct ImmValue {
    ImmValue()
        : ty(IMM_I32), val() {}
    ImmValue(bool flag)
        : ty(IMM_I1), val((long long)flag) {}
    ImmValue(ImmType ty)
        : ty(ty), val() {}
    ImmValue(int val)
        : ty(IMM_I32), val((long long) val) {}
    ImmValue(long long val, ImmType ty = IMM_I64)
        : ty(ty), val(val) {}
    ImmValue(unsigned int val)
        : ty(IMM_U32), val((unsigned long long) val) {}
    ImmValue(unsigned long long val, ImmType ty = IMM_U64)
        : ty(ty), val(val) {}
    ImmValue(float val)
        : ty(IMM_F32), val(val) {}
    ImmValue(double val)
        : ty(IMM_F64), val(val) {}

    ImmType ty;
    ImmValueOnly val;

    ImmValue cast_to(ImmType new_ty) const;

    operator bool() const;
    ImmValue operator + (ImmValue o) const;
    ImmValue operator - (ImmValue o) const;
    ImmValue operator * (ImmValue o) const;
    ImmValue operator / (ImmValue o) const;
    ImmValue operator % (ImmValue o) const;
    ImmValue operator && (ImmValue o) const;
    ImmValue operator || (ImmValue o) const;
    ImmValue operator & (ImmValue o) const;
    ImmValue operator | (ImmValue o) const;
    ImmValue operator < (ImmValue o) const;
    ImmValue operator > (ImmValue o) const;
    ImmValue operator <= (ImmValue o) const;
    ImmValue operator >= (ImmValue o) const;
    ImmValue operator == (ImmValue o) const;
    ImmValue operator != (ImmValue o) const;
    ImmValue operator ! () const;
};

ImmType join_imm_type(ImmType a, ImmType b);

typedef Vector<ImmValue> ImmValues;
typedef Opt<ImmValue> ImmOrVoid;

const Map<String, ImmType> gSymToImmType {
#define ENTRY(x, y, z) { #z, IMM_##y },
    IMM_TYPE_TABLE
#undef ENTRY
};

#define ENTRY(x, y, z) #z,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY
#undef IMM_TYPE_TABLE

struct ImmTypedSym
{
    ImmTypedSym(Symbol sym, ImmType tr, bool is_const = false);

    Symbol sym;
    ImmType tr;
    bool is_const;
};
