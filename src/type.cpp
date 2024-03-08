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
    sprintf(tmp, "[%llu x %s]", elem_count, elem_type->type_name());
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

bool is_signed_imm_type(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_signed_imm_type(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

bool is_float(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return is_float(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

int bits_of_type(pType tr)
{
    if(tr->type_type() == TYPE_BASIC_TYPE)
        return bits_of_type(std::static_pointer_cast<BasicType>(tr)->ty);
    return false;
}

void Memory::realloc(size_t len)
{
    this->len = len;
    auto j = new int8_t[len];
    memset(j, 0, sizeof(len));
    this->mem = RawMemory(j);
}
