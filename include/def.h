// 整个项目共有的基本常量和数据类型
// 包括使用的模板类型、目标机的位数等信息

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

// 目标机的指针长度
#define ARCH_BYTES 8

// 带有错误信息的 assert 宏包装
// 注意，理论上所有使用 my_assert 的地方都应该改为 Exception
#define my_assert(x, y) assert(x)

// 一些模板类型的别名，便于更换为其他数据类型

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

template<typename T, typename U>
using Pair = std::pair<T, U>;

// Immediate 表示在该项目在常规计算中使用的数字类型
// 很少使用
typedef long long Immediate;

typedef float Float32;
typedef double Float64;

// 使用的字符串
// 注意，Symbol 是 C 风格字符串
// String 为 C++ 风格字符串
typedef const char* Symbol;
typedef std::string String;
typedef Pointer<String> pString;

// to_symbol 函数内部存在一个内存池
// 将所有的 String 拷贝为 C 风格字符串
// 并且保证在程序结束后销毁所有开辟的 C 风格字符串
Symbol to_symbol(String s);

// 表示代码的数据结构
struct Code {
    // 代码本体
    pString p_code;
    // 代码的名称
    String file_name;
};

typedef Pointer<Code> pCode;

// 所有异常的基类
struct Exception {
    Exception(size_t id, Symbol obj, Symbol msg)
        : id(id), object(obj), message(msg) { }
    size_t id;
    Symbol object;
    Symbol message;
};

// 接下来的部分与运行时类型的存放有关
// 未来将被废弃

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

