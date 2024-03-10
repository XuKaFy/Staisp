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

bool same_type(pType a1, pType a2)
{
    return strcmp(a1->type_name(), a2->type_name()) == 0;
}

pType join_type(pType a1, pType a2)
{
    my_assert(a1 && a2, "how");
    if(is_pointer(a1) && is_imm_type(a2))
        return make_basic_type(ARCH_USED_POINTER_TYPE, false);
    if(is_pointer(a2) && is_imm_type(a1))
        return make_basic_type(ARCH_USED_POINTER_TYPE, false);
    if(a1->type_type() != a2->type_type()) {
        return pType();
    }
    switch(a1->type_type()) {
    case TYPE_BASIC_TYPE:
        return make_basic_type(join_imm_type(std::static_pointer_cast<BasicType>(a1)->ty,
                std::static_pointer_cast<BasicType>(a2)->ty), false);
    default: break;
    }
    return pType();
}

bool castable(pType from, pType to)
{
    if(same_type(from, to)) return true;
    if(is_pointer(from) && is_imm_type(to)) {
        return true;
    }
    if(is_pointer(to) && is_imm_type(from)) {
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

bool is_imm_type(pType p)
{
    return p->type_type() == TYPE_BASIC_TYPE;
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

bool is_signed_type(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_signed_type(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

bool is_float(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_float(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

bool is_integer(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_imm_integer(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

size_t bits_of_imm_type(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
    switch(tr->type_type()) {
    case TYPE_BASIC_TYPE:
        return bits_of_imm_type(std::static_pointer_cast<BasicType>(tr)->ty);
    case TYPE_COMPOUND_TYPE:
        return std::static_pointer_cast<CompoundType>(tr)->length();
    default: break;
    }
    return 0;
}

void Memory::realloc(size_t len)
{
    this->len = len;
    auto j = new int8_t[len];
    memset(j, 0, sizeof(len));
    this->mem = RawMemory(j);
}

Pointer<PointerType> to_pointer(pType p)
{
    auto j = std::static_pointer_cast<PointerType>(p);
    my_assert(j, "how");
    return j;
}

pType to_pointed_type(pType p)
{
    return to_pointer(p)->pointed_type;
}

Pointer<ArrayType> to_array(pType p)
{
    auto j = std::static_pointer_cast<ArrayType>(p);
    my_assert(j, "how");
    return j;
}

pType to_elem_type(pType p)
{
    return to_array(p)->elem_type;
}
