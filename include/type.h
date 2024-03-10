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
//
// 注意任意类型都会具有以下属性：
//
// * type_type：这个类型是复合类型还是基本类型
// * type_name：类型的字符串形式
// * length：该类型所占据的字节数
//
// 由于不是所有类型均有 CONST 形式，因此在抽象类型当中不体现
struct Type {
    Type() { }

    virtual TypeType type_type() const { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const { return "void"; }
    virtual size_t length() const { return 0; }
};

typedef Pointer<Type> pType;

// 表达具有任意类型的标识符
struct TypedSym
{
    TypedSym(Symbol sym, pType tr)
        : sym(sym), tr(tr) { }

    Symbol sym;
    pType tr;
};

// 基础类型 ImmType 的封装
// 使用继承自 Type 的 BasicType 进行表达
// 注意，基本类型区分是否为 CONST
struct BasicType : public Type
{
    BasicType(ImmType ty, bool is_const)
        : Type(), ty(ty), is_const(is_const) { }

    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const override;
    virtual size_t length() const override { return bytes_of_imm_type(ty); }
    ImmType ty;
    bool is_const;
};

// 暂定的复合类型只有三种：指针、数组、结构体
enum CompoundTypeType {
    COMPOUND_TYPE_POINTER,
    COMPOUND_TYPE_STRUCT,
    COMPOUND_TYPE_ARRAY,
};

struct CompoundType : public Type {
    CompoundType()
        : Type() { }

    virtual TypeType type_type() const override { return TYPE_COMPOUND_TYPE; }
    virtual CompoundTypeType compound_type_type() const = 0;
};

struct StructType : public CompoundType {
    StructType(Vector<TypedSym> elems, bool is_const)
        : CompoundType(), elems(elems), is_const(is_const) { }
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_STRUCT; }
    virtual size_t length() const override {
        size_t ans = 0;
        for(auto i : elems) {
            ans += i.tr->length();
        }
        return ans;
    }

    Vector<TypedSym> elems;
    bool is_const;
};

struct PointerType : public CompoundType {
    PointerType(pType ty, bool is_const)
        : CompoundType(), pointed_type(ty), is_const(is_const) { }

    virtual Symbol type_name() const override;
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    virtual size_t length() const override { return ARCH_BYTES; }

    pType pointed_type;
    bool is_const;
};

struct ArrayType : public CompoundType {
    ArrayType(pType elem_type, size_t elem_count)
        : CompoundType(), elem_type(elem_type), elem_count(elem_count) { }

    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_ARRAY; }
    virtual Symbol type_name() const override;
    virtual size_t length() const override { return elem_type->length() * elem_count; }
    
    pType elem_type;
    size_t elem_count;
};

// 创建类型类型的 helper
pType make_basic_type(ImmType ty, bool is_const);
pType make_array_type(pType ty, size_t count);
pType make_pointer_type(pType ty, bool is_const);

// 与类型相关的 helper
pType join_type(pType a1, pType a2);
bool is_same_type(pType a1, pType a2);
bool is_castable(pType from, pType to);
bool is_pointer(pType p);
bool is_array(pType p);
bool is_struct(pType p);
bool is_basic_type(pType p);
bool is_const_type(pType p);

// 注意，若不是基本类型，以下所有的函数均返回 false
bool is_signed_type(pType tr);
bool is_float(pType tr);
bool is_integer(pType tr);
size_t bytes_of_type(pType tr);

// 对复合类型的解包的 convertor
Pointer<PointerType> to_pointer_type(pType p);
pType to_pointed_type(pType p);
Pointer<ArrayType> to_array_type(pType p);
pType to_elem_type(pType p);
Pointer<StructType> to_struct_type(pType p);
Pointer<BasicType> to_basic_type(pType p);
