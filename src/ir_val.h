#pragma once

#include "def.h"

namespace Ir {

#define IMM_TYPE_TABLE \
    ENTRY(0, VOID, void) \
    ENTRY(1, I8, i8) \
    ENTRY(2, I16, i16) \
    ENTRY(3, I32, i32) \
    ENTRY(4, I64, i64)

enum ImmType {
#define ENTRY(x, y, z) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

#define ENTRY(x, y, z) #z,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY

#undef IMM_TYPE_TABLE

struct Val {
    Symbol print();
    virtual Symbol print_impl() const = 0;

    Symbol str_form { nullptr };
};

typedef Pointer<Val> pVal;

} // namespace ir