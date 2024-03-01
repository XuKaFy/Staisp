#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <memory>
#include <cstring>

#include <variant>

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
