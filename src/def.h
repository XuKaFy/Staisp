#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <memory>
#include <cstring>
#include <cassert>

#include <variant>

#define my_assert(x, y) assert(x)

template<typename T>
using Pointer = std::shared_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T, typename U>
using Map = std::map<T, U>;

typedef long long Immediate;
typedef const char* Symbol;
typedef std::string String;

Symbol to_symbol(String s);

#define IMM_TYPE_TABLE \
    ENTRY(0, I1, i1) \
    ENTRY(1, I8, i8) \
    ENTRY(2, I16, i16) \
    ENTRY(3, I32, i32) \
    ENTRY(4, I64, i64)

enum ImmType {
#define ENTRY(x, y, z) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

ImmType join_type(ImmType a, ImmType b);

#define ENTRY(x, y, z) #z,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY

#undef IMM_TYPE_TABLE

struct TypedSym
{
    Symbol sym;
    ImmType tr;
};
