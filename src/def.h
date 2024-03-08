#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <memory>
#include <cstring>
#include <cassert>
#include <list>
#include <utility>
#include <optional>
#include <cstdint>
#include <climits>

#include <variant>

#define ARCH_BYTES 8

#define my_assert(x, y) assert(x)

template<typename T>
using Pointer = std::shared_ptr<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T, typename U>
using Map = std::map<T, U>;

template<typename T>
using Opt = std::optional<T>;

typedef long long Immediate;
typedef double Float32;
typedef double Float64;
typedef const char* Symbol;
typedef std::string String;
typedef Pointer<String> pString;

template<typename T, typename U>
using Pair = std::pair<T, U>;

Symbol to_symbol(String s);

struct Code {
    pString p_code;
    String file_name;
};

typedef Pointer<Code> pCode;

typedef Pointer<int8_t> RawMemory;

struct Memory {
    Memory()
        : len(0), mem() {}
    Memory(size_t len)
        : len(len), mem(RawMemory(new int8_t[len])) {}
    void realloc(size_t len);

    size_t len;
    RawMemory mem;
};

struct MemoryRef
{
    MemoryRef(Memory mem)
        : begin(0), len(mem.len), mem(mem.mem) { }
    size_t begin;
    size_t len;
    RawMemory mem;
};
