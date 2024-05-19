// 与基本类型相关的所有类和基本常量
// 基本类型大致有：
// 
// * i64/32/16/8/1：有符号类型
// * u64/32/16/8/1：无符号类型
// * f64/f32：浮点类型
//
// 其中，有符号、无符号数的唯一区别，在于运算时所需要的指令
// 其余情况均一致
// 而对于浮点数，则有不同的计算指令

#pragma once

#include "def.h"

#define IMM_TYPE_TABLE \
    ENTRY(0, I1,    i1,   i1) \
    ENTRY(1, U1,    u1,   i1) \
    ENTRY(2, I8,    i8,   i8) \
    ENTRY(3, U8,    i8,   i8) \
    ENTRY(4, I16,   i16,  i16) \
    ENTRY(5, U16,   u16,  i16) \
    ENTRY(6, I32,   i32,  i32) \
    ENTRY(7, U32,   u32,  i32) \
    ENTRY(8, F32,   f32,  float) \
    ENTRY(9, I64,   i64,  i64) \
    ENTRY(10, U64,  u64,  i64) \
    ENTRY(11, F64,  f64,  double)

// 基本类型指示
enum ImmType {
#define ENTRY(x, y, z, a) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

// 在目标机中，使用的指针类型为 IMM_U64
#define ARCH_USED_POINTER_TYPE IMM_U64

// 此全局 Map 将小写类型转换为基本类型指示
const Map<String, ImmType> gSymToImmType {
#define ENTRY(x, y, z, a) { #z, IMM_##y },
    IMM_TYPE_TABLE
#undef ENTRY
};

// 此全局 Map 通过 IMM 编号寻找其**到 IR 后**的类型
// 例如，不论是 IMM_U64 还是 IMM_I64
// 均映射为 i64
// 对于浮点数，IMM_F64 映射为 double，IMM_F32 映射为 float
#define ENTRY(x, y, z, a) #a,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY
#undef IMM_TYPE_TABLE

// 与 Imm 类型有关的 utilities
bool is_imm_signed(ImmType tr);
bool is_imm_float(ImmType tr);
bool is_imm_integer(ImmType t);
size_t bytes_of_imm_type(ImmType tr);
ImmType join_imm_type(ImmType a, ImmType b);

// 此结构体存放基本类型的数据**本身**
// 而不携带和类型有关的信息
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
    // 内部恒用一个长为 8 字节的联合存放一切基本类型数据
    union {
        long long ival;
        unsigned long long uval;
        float f32val;
        double f64val;
    };
};

// 此即为基本类型实例类
// 包含值与类型两重信息
struct ImmValue {
    ImmValue()
        : ty(IMM_I32), val() {}
    // 注意，对于 bool 类型而言，其被翻译为 IMM_I1
    ImmValue(bool flag)
        : ty(IMM_I1), val((long long)flag) {}
    ImmValue(ImmType ty)
        : ty(ty), val() {
        switch(ty) {
        case IMM_I1:
        case IMM_I8:
        case IMM_I16:
        case IMM_I32:
        case IMM_I64: val.ival = 0; break;
        case IMM_U1:
        case IMM_U8:
        case IMM_U16:
        case IMM_U32:
        case IMM_U64: val.uval = 0; break;
        // 由于浮点数的 0 的定义与整数不同，所以需要额外判断
        case IMM_F32: val.f32val = 0; break;
        case IMM_F64: val.f64val = 0; break;
        }
    }
    ImmValue(int val)
        : ty(IMM_I32), val((long long) val) {}
    ImmValue(long long val, ImmType ty = IMM_I64) // 构造时支持降格，例如代入 IMM_I32/16/8/1
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

    // cast_to 利用**重新构造**一个 ImmValue 实现
    // 例如，将 F64 类型的 0 转化为 I64
    // 其值仍然是 0，而不是浮点数标准下的 0
    ImmValue cast_to(ImmType new_ty) const;

    operator bool() const;
    ImmValue operator ^ (ImmValue o) const;
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

    Symbol print() const;
};

typedef Vector<ImmValue> ImmValues;
typedef Opt<ImmValue> ImmOrVoid;

// TypedSym 类型即限定某个标识符的类型如何
struct ImmTypedSym
{
    ImmTypedSym(Symbol sym, ImmType tr, bool is_const = false);

    Symbol sym;
    ImmType tr;
    bool is_const;
};
