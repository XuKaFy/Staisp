#include "type.h"

#include <memory>

#include <utility>
#include "def.h"

String BasicType::type_name() const { return gImmName[ty]; }

String PointerType::type_name() const {
    return pointed_type->type_name() + "*";
}

String ArrayType::type_name() const {
    static char tmp[256];
    sprintf(tmp, "[%llu x %s]", static_cast<unsigned long long>(elem_count),
            elem_type->type_name().c_str());
    return tmp;
}

pType make_void_type() { return std::make_shared<Type>(); }

pType make_ir_type(VoidIrType ir_ty) { return pType(new IrType(ir_ty)); }

pType make_basic_type(ImmType ty) { return pType(new BasicType(ty)); }

pType make_function_type(pType ret_type, Vector<pType> arg_type) {
    return pType(new FunctionType(std::move(ret_type), std::move(arg_type)));
}

pType make_pointer_type(pType ty) { return pType(new PointerType(std::move(ty))); }

pType make_array_type(pType ty, size_t count) {
    return pType(new ArrayType(std::move(ty), count));
}

pType join_type(pType a1, const pType& a2) {
    my_assert(a1 && a2, "how");
    if (is_same_type(a1, a2)) {
        return a1;
}
    // 指针可以和基本整型进行运算，并且最后的结果是 ARCH_USED_POINTER_TYPE
    // 一般来说，会变成 IMM_U64
    if (is_pointer(a1) && is_basic_type(a2) &&
        is_imm_integer(to_basic_type(a2)->ty)) {
        return make_basic_type(ARCH_USED_POINTER_TYPE);
}
    if (is_pointer(a2) && is_basic_type(a1) &&
        is_imm_integer(to_basic_type(a1)->ty)) {
        return make_basic_type(ARCH_USED_POINTER_TYPE);
}
    // 其他情况下，认为结构体和数组和基本类型和指针之间不可运算
    if (a1->type_type() != a2->type_type()) {
        return {};
    }
    switch (a1->type_type()) {
    case TYPE_BASIC_TYPE:
        return make_basic_type(
            join_imm_type(to_basic_type(a1)->ty, to_basic_type(a2)->ty));
    default:
        break;
    }
    return {};
}

// 只要两个类型的字符表达一致，就一定是同一类型
bool is_same_type(const pType& a1, const pType& a2) {
    if (!a1) {
        throw Exception(1, "is_same_type", "arg1 is empty");
    }
    if (!a2) {
        throw Exception(2, "is_same_type", "arg2 is empty");
    }
    return a1->type_name() == a2->type_name();
}

bool is_castable(const pType& from, const pType& to) {
    if (is_same_type(from, to)) {
        return true;
}
    // 基本类型之间可以互相转换
    if (is_basic_type(from) && is_basic_type(to)) {
        return true;
}
    // 指针之间可以互相转换
    if (is_pointer(from) && is_pointer(to)) {
        return true;
}
    // 注意，若涉及转换，那么指针只能和 64 位基本整型进行互相转化
    if (is_pointer(from) && is_basic_type(to) &&
        is_imm_integer(to_basic_type(to)->ty) &&
        bytes_of_imm_type(to_basic_type(to)->ty) == ARCH_BYTES) {
        return true;
    }
    if (is_pointer(to) && is_basic_type(from) &&
        is_imm_integer(to_basic_type(from)->ty) &&
        bytes_of_imm_type(to_basic_type(from)->ty) == ARCH_BYTES) {
        return true;
    }
    return false;
}

bool is_pointer(const pType& p) {
    if (p->type_type() != TYPE_COMPOUND_TYPE) {
        return false;
}
    if (std::dynamic_pointer_cast<CompoundType>(p)->compound_type_type() !=
        COMPOUND_TYPE_POINTER) {
        return false;
}
    return true;
}

bool is_array(const pType& p) {
    if (p->type_type() != TYPE_COMPOUND_TYPE) {
        return false;
}
    if (std::dynamic_pointer_cast<CompoundType>(p)->compound_type_type() !=
        COMPOUND_TYPE_ARRAY) {
        return false;
}
    return true;
}

bool is_struct(const pType& p) {
    if (p->type_type() != TYPE_COMPOUND_TYPE) {
        return false;
}
    if (std::dynamic_pointer_cast<CompoundType>(p)->compound_type_type() !=
        COMPOUND_TYPE_STRUCT) {
        return false;
}
    return true;
}

bool is_basic_type(const pType& p) { return p->type_type() == TYPE_BASIC_TYPE; }

bool is_signed_type(const pType& ty) {
    if (ty->type_type() == TYPE_BASIC_TYPE) {
        return is_imm_signed(to_basic_type(ty)->ty);
}
    return false;
}

bool is_float(const pType& ty) {
    if (ty->type_type() == TYPE_BASIC_TYPE) {
        return is_imm_float(to_basic_type(ty)->ty);
}
    return false;
}

bool is_integer(const pType& ty) {
    if (ty->type_type() == TYPE_BASIC_TYPE) {
        return is_imm_integer(to_basic_type(ty)->ty);
}
    return false;
}

size_t bytes_of_type(const pType& ty) {
    switch (ty->type_type()) {
    case TYPE_BASIC_TYPE:
        return bytes_of_imm_type(to_basic_type(ty)->ty);
    case TYPE_COMPOUND_TYPE:
        if (is_struct(ty)) {
            return to_struct_type(ty)->length();
}
        if (is_pointer(ty)) {
            return to_pointer_type(ty)->length();
}
        if (is_array(ty)) {
            return to_array_type(ty)->length();
}
        //  my_assert(false, "how");
    default:
        break;
    }
    return 0;
}

Pointer<PointerType> to_pointer_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<PointerType>(p);
    if (!j) {
        throw Exception(1, "to_pointer_type", "not castable");
}
    return j;
}

pType to_pointed_type(const pType& p) {
    if (!is_pointer(p)) {
        throw Exception(1, "to_pointed_type", "not a pointer");
    }
    return to_pointer_type(p)->pointed_type;
}

Pointer<ArrayType> to_array_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<ArrayType>(p);
    if (!j) {
        throw Exception(1, "to_array_type", "not castable");
}
    return j;
}

pType to_elem_type(const pType& p) { return to_array_type(p)->elem_type; }

Pointer<StructType> to_struct_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<StructType>(p);
    if (!j) {
        throw Exception(1, "to_struct_type", "not castable");
}
    return j;
}

Pointer<BasicType> to_basic_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<BasicType>(p);
    if (!j) {
        throw Exception(1, "to_basic_type", "not castable");
    }
    return j;
}

Pointer<FunctionType> to_function_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<FunctionType>(p);
    if (!j) {
        throw Exception(1, "to_function_type", "not castable");
}
    return j;
}

VoidIrType to_ir_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<IrType>(p);
    if (!j) {
        throw Exception(1, "to_ir_type", "not castable");
}
    return j->ir_ty;
}

bool is_ir_type(const pType& p, VoidIrType ty) {
    auto j = std::dynamic_pointer_cast<IrType>(p);
    if (!j) {
        return false;
}
    return j->ir_ty == ty;
}

bool is_ir_type(const pType& p) {
    auto j = std::dynamic_pointer_cast<IrType>(p);
    return static_cast<bool>(j);
}
