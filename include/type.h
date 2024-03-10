// 与复杂类型相关的所有类和常量
// 类型分为两大类：BasicType 与 Compound Type，即基本类型和复合类型
// 而 Type 类可以同时表达这两大类类型
//
// 对于 Compound Type 其下还有三种复合类型
// * PointerType：指针类型，作为一个指向某已知类型的指针
// * ArrayType：数组类型，作为一个元素全为已知类型的数组
// * StructType：结构体类型，作为内部存放若干已知类型的结构体
// 
// 复合类型的本质是对基本类型或另一已知的复合类型进行一次包装
// 在使用时进行解包

#pragma once

#include "def.h"
#include "imm.h"

// 类型的“类型”
// 即该类型是基本类型还是复合类型
enum TypeType {
    TYPE_BASIC_TYPE,
    TYPE_COMPOUND_TYPE,
};

// 表达所有类型的类
// 由于在 IR 或者汇编中都不存在 CONST 的概念
// 因此 is_const 只在抽象程度较高的编译器前端中具有意义
// 注意，不是所有的类的 is_const 在前端中具有意义，例如数组
// 因为数组本来就不可直接赋值，因此 CONST 无意义
struct Type {
    Type(bool is_const = false)
        : is_const(is_const) { }

    virtual TypeType type_type() const { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const { return "void"; }
    virtual size_t length() const { return 0; }
    bool is_const;
};

typedef Pointer<Type> pType;

bool same_type(pType a1, pType a2);
pType join_type(pType a1, pType a2);
bool castable(pType from, pType to);
bool is_pointer(pType p);
bool is_imm_type(pType p);

struct TypedSym
{
    TypedSym(Symbol sym, pType tr)
        : sym(sym), tr(tr) { }

    Symbol sym;
    pType tr;
};

struct BasicType : public Type
{
    BasicType(ImmType ty, bool is_const)
        : Type(is_const), ty(ty) { }

    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const override;
    virtual size_t length() const override { return bits_of_imm_type(ty); }
    ImmType ty;
};

pType make_basic_type(ImmType ty, bool is_const);

enum CompoundTypeType {
    COMPOUND_TYPE_POINTER,
    COMPOUND_TYPE_STRUCT,
    COMPOUND_TYPE_ARRAY,
};

struct CompoundType : public Type {
    CompoundType(bool is_const)
        : Type(is_const) { }

    virtual TypeType type_type() const override { return TYPE_COMPOUND_TYPE; }
    virtual CompoundTypeType compound_type_type() const = 0;
};

struct StructType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_STRUCT; }

    Vector<TypedSym> elems;
};

struct PointerType : public CompoundType {
    PointerType(pType ty, bool is_const)
        : CompoundType(is_const), pointed_type(ty) { }

    virtual Symbol type_name() const override;
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    virtual size_t length() const { return ARCH_BYTES; }

    pType pointed_type;
};

pType make_pointer_type(pType ty, bool is_const);

struct ArrayType : public CompoundType {
    ArrayType(pType elem_type, size_t elem_count)
        : CompoundType(false), elem_type(elem_type), elem_count(elem_count) { }

    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_ARRAY; }
    virtual Symbol type_name() const override;
    virtual size_t length() const { return elem_type->length() * elem_count; }
    
    pType elem_type;
    size_t elem_count;
};

pType make_array_type(pType ty, size_t count);

bool is_signed_type(pType tr);
bool is_float(pType tr);
size_t bits_of_imm_type(pType tr);
bool is_integer(pType tr);

bool is_array(pType tr);

Pointer<PointerType> to_pointer(pType p);
pType to_pointed_type(pType p);

Pointer<ArrayType> to_array(pType p);
pType to_elem_type(pType p);
