#include "type.h"

Symbol BasicType::type_name() const
{
    return gImmName[ty];
}

Symbol PointerType::type_name() const
{
    return to_symbol(String(pointed_type->type_name()) + "*");
}

Symbol ArrayType::type_name() const
{
    static char tmp[256];
    sprintf(tmp, "[%llu x %s]", (unsigned long long) elem_count, elem_type->type_name());
    return to_symbol(String(tmp));
}

pType make_basic_type(ImmType ty, bool is_const)
{
    return pType(new BasicType(ty, is_const));
}

pType make_pointer_type(pType ty, bool is_const)
{
    return pType(new PointerType(ty, is_const));
}

pType make_array_type(pType ty, size_t count)
{
    return pType(new ArrayType(ty, count));
}

pType join_type(pType a1, pType a2)
{
    my_assert(a1 && a2, "how");
    // 指针可以和基本整型进行运算，并且最后的结果是 ARCH_USED_POINTER_TYPE
    // 一般来说，会变成 IMM_U64
    if(is_pointer(a1) && is_basic_type(a2) && is_imm_integer(to_basic_type(a2)->ty))
        return make_basic_type(ARCH_USED_POINTER_TYPE, false);
    if(is_pointer(a2) && is_basic_type(a1) && is_imm_integer(to_basic_type(a1)->ty))
        return make_basic_type(ARCH_USED_POINTER_TYPE, false);
    // 其他情况下，认为结构体和数组和基本类型和指针之间不可运算
    if(a1->type_type() != a2->type_type()) {
        return pType();
    }
    switch(a1->type_type()) {
    case TYPE_BASIC_TYPE:
        return make_basic_type(join_imm_type(to_basic_type(a1)->ty,
                to_basic_type(a2)->ty), false);
    default: break;
    }
    return pType();
}

// 只要两个类型的字符表达一致，就一定是同一类型
bool is_same_type(pType a1, pType a2)
{
    return strcmp(a1->type_name(), a2->type_name()) == 0;
}

bool is_castable(pType from, pType to)
{
    if(is_same_type(from, to)) return true;
    // 基本类型之间可以互相转换
    if(is_basic_type(from) && is_basic_type(to))
        return true;
    // 注意，若涉及转换，那么指针只能和 64 位基本整型进行互相转化
    if(is_pointer(from) && is_basic_type(to) && is_imm_integer(to_basic_type(to)->ty) && bytes_of_imm_type(to_basic_type(to)->ty) == ARCH_BYTES) {
        return true;
    }
    if(is_pointer(to) && is_basic_type(from) && is_imm_integer(to_basic_type(from)->ty) && bytes_of_imm_type(to_basic_type(from)->ty) == ARCH_BYTES) {
        return true;
    }
    return false;
}

bool is_pointer(pType p)
{
    if(p->type_type() != TYPE_COMPOUND_TYPE) return false;
    if(std::static_pointer_cast<CompoundType>(p)->compound_type_type() != COMPOUND_TYPE_POINTER)
        return false;
    return true;
}

bool is_array(pType p)
{
    if(p->type_type() != TYPE_COMPOUND_TYPE) return false;
    if(std::static_pointer_cast<CompoundType>(p)->compound_type_type() != COMPOUND_TYPE_ARRAY)
        return false;
    return true;
}

bool is_struct(pType p)
{
    if(p->type_type() != TYPE_COMPOUND_TYPE) return false;
    if(std::static_pointer_cast<CompoundType>(p)->compound_type_type() != COMPOUND_TYPE_STRUCT)
        return false;
    return true;
}

bool is_basic_type(pType p)
{
    return p->type_type() == TYPE_BASIC_TYPE;
}

bool is_const_type(pType p)
{
    if(is_basic_type(p))
        return to_basic_type(p)->is_const;
    if(is_pointer(p))
        return to_pointer_type(p)->is_const;
    return false;
}

bool is_signed_type(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_signed(to_basic_type(tr)->ty);
    return false;
}

bool is_float(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_float(to_basic_type(tr)->ty);
    return false;
}

bool is_integer(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_integer(to_basic_type(tr)->ty);
    return false;
}

size_t bytes_of_type(pType tr)
{
    switch(tr->type_type()) {
    case TYPE_BASIC_TYPE:
        return bytes_of_imm_type(to_basic_type(tr)->ty);
    case TYPE_COMPOUND_TYPE:
        if(is_struct(tr))
            return to_struct_type(tr)->length();
        if(is_pointer(tr))
            return to_pointer_type(tr)->length();
        if(is_array(tr))
            return to_array_type(tr)->length();
       //  my_assert(false, "how");
    default: break;
    }
    return 0;
}

Pointer<PointerType> to_pointer_type(pType p)
{
    auto j = std::static_pointer_cast<PointerType>(p);
    if(!j)
        throw Exception(1, "to_pointer_type", "not castable");
    return j;
}

pType to_pointed_type(pType p)
{
    return to_pointer_type(p)->pointed_type;
}

Pointer<ArrayType> to_array_type(pType p)
{
    auto j = std::static_pointer_cast<ArrayType>(p);
    if(!j)
        throw Exception(1, "to_array_type", "not castable");
    return j;
}

pType to_elem_type(pType p)
{
    return to_array_type(p)->elem_type;
}

Pointer<StructType> to_struct_type(pType p)
{
    auto j = std::static_pointer_cast<StructType>(p);
    if(!j)
        throw Exception(1, "to_struct_type", "not castable");
    return j;
}

Pointer<BasicType> to_basic_type(pType p)
{
    auto j = std::static_pointer_cast<BasicType>(p);
    if(!j)
        throw Exception(1, "to_basic_type", "not castable");
    return j;
}
