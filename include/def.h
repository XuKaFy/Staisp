// 整个项目共有的基本常量和数据类型
// 包括使用的模板类型、目标机的位数等信息

#pragma once

#include <cassert>
#include <climits>
#include <cstring>
#include <list>
#include <memory>
#include <optional>
#include <unordered_set>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

// 目标机的指针长度
#define ARCH_BYTES 8

// 带有错误信息的 assert 宏包装
// 注意，理论上所有使用 my_assert 的地方都应该改为 Exception
#define my_assert(x, y) assert(x)

// 一些模板类型的别名，便于更换为其他数据类型

template <typename T> using Pointer = std::shared_ptr<T>;

template <typename T> using WeakPointer = std::weak_ptr<T>;

template <typename T> using List = std::list<T>;

template <typename T> using Vector = std::vector<T>;

template <typename T> using Set = std::unordered_set<T>;

template <typename T> using Stack = std::stack<T>;

template <typename T> using Queue = std::queue<T>;

template <typename T, typename U> using Map = std::unordered_map<T, U>;

template <typename T> using Opt = std::optional<T>;

template <typename T, typename U> using Pair = std::pair<T, U>;

using String = std::string;
using pString = Pointer<String>;

// 表示代码的数据结构
struct Code {
    // 代码本体
    pString p_code;
    // 代码的名称
    String file_name;
};

using pCode = Pointer<Code>;

pCode make_code(String code, String file_name);

// 所有异常的基类
struct Exception {
    Exception(size_t id, String obj, String msg)
        : id(id), object(std::move(std::move(obj))),
          message(std::move(std::move(msg))) {}
    size_t id;
    String object;
    String message;
};
