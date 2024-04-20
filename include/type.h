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
//
// 还有一类特殊类型 IrType，用于当某个 Ir 没有代表某个寄存器（也就是没有类型）时
// 它是哪种 Ir

#pragma once

#include "def.h"
#include "imm.h"

// 类型的“类型”
// 即该类型是基本类型还是复合类型
// 为了便于 IR 的书写
// 这里为 IR 准备了一个 Type
// * IR_TYPE
enum TypeType {
    TYPE_VOID_TYPE,
    TYPE_IR_TYPE,
    TYPE_BASIC_TYPE,
    TYPE_COMPOUND_TYPE,
};

// 表达所有类型的类
// 注意任意类型都会具有以下属性：
//
// * type_type：这个类型是复合类型还是基本类型
// * type_name：类型的字符串形式
// * length：该类型所占据的字节数
struct Type {
    Type() { }

    virtual TypeType type_type() const { return TYPE_VOID_TYPE; }
    virtual Symbol type_name() const { return "void"; }
    virtual size_t length() const { return 0; }
};

typedef Pointer<Type> pType;

// 表达具有任意类型的标识符
struct TypedSym
{
    TypedSym(Symbol sym, pType ty)
        : sym(sym), ty(ty) { }

    Symbol sym;
    pType ty;
};

enum VoidIrType {
    IR_STORE,
    IR_BR,
    IR_BR_COND,
    IR_RET,
    IR_LABEL,
};

// 为表达 Ir 指令类型所准备的类
struct IrType : public Type {
    IrType(VoidIrType ty)
        : Type(), ir_ty(ty) { }

    virtual TypeType type_type() const override { return TYPE_IR_TYPE; }
    virtual Symbol type_name() const override { return "ir"; }
    virtual size_t length() const override { return 0; }

    VoidIrType ir_ty;
};

// 基础类型 ImmType 的封装
// 使用继承自 Type 的 BasicType 进行表达
struct BasicType : public Type
{
    BasicType(ImmType ty)
        : Type(), ty(ty) { }

    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const override;
    virtual size_t length() const override { return bytes_of_imm_type(ty); }
    ImmType ty;
};

// 函数类型
struct FunctionType : public Type
{
    FunctionType(pType ret_type, Vector<pType> arg_type)
        : Type(), ret_type(ret_type), arg_type(arg_type) { }

    virtual TypeType type_type() const override { return ret_type->type_type(); }
    virtual Symbol type_name() const override { return ret_type->type_name(); }
    virtual size_t length() const override { return ret_type->length(); }

    pType ret_type;
    Vector<pType> arg_type;
};
typedef Pointer<FunctionType> pFunctionType;

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
    StructType(Vector<TypedSym> elems)
        : CompoundType(), elems(elems) { }
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_STRUCT; }
    virtual size_t length() const override {
        size_t ans = 0;
        for(auto i : elems) {
            ans += i.ty->length();
        }
        return ans;
    }

    Vector<TypedSym> elems;
};

struct PointerType : public CompoundType {
    PointerType(pType ty)
        : CompoundType(), pointed_type(ty) { }

    virtual Symbol type_name() const override;
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    virtual size_t length() const override { return ARCH_BYTES; }

    pType pointed_type;
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
pType make_void_type();
pType make_ir_type(VoidIrType ir_ty);
pType make_basic_type(ImmType ty);
pType make_function_type(pType ret_type, Vector<pType> arg_type);
pType make_array_type(pType ty, size_t count);
pType make_pointer_type(pType ty);

// 与类型相关的 helper
pType join_type(pType a1, pType a2);
bool is_same_type(pType a1, pType a2);
bool is_castable(pType from, pType to);
bool is_pointer(pType p);
bool is_array(pType p);
bool is_struct(pType p);
bool is_basic_type(pType p);

// 注意，若不是基本类型，以下所有的函数均返回 false
bool is_signed_type(pType ty);
bool is_float(pType ty);
bool is_integer(pType ty);
size_t bytes_of_type(pType ty);

// 对复合类型的解包的 convertor
Pointer<PointerType> to_pointer_type(pType p);
pType to_pointed_type(pType p);
Pointer<ArrayType> to_array_type(pType p);
pType to_elem_type(pType p);
Pointer<StructType> to_struct_type(pType p);
Pointer<BasicType> to_basic_type(pType p);
Pointer<FunctionType> to_function_type(pType p);
VoidIrType to_ir_type(pType p);

bool is_ir_type(pType p, VoidIrType ty);
bool is_ir_type(pType p);
