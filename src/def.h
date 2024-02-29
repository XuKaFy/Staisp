#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <memory>

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T, typename U>
using Map = std::map<T, U>;


typedef long long Integer;
typedef std::string String;

template<typename T>
using Pointer = std::shared_ptr<T>;

