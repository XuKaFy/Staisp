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

#include <variant>

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
typedef Vector<Immediate> Immediates;
typedef const char* Symbol;
typedef std::string String;
typedef Pointer<String> pString;

typedef Opt<Immediate> ImmOrVoid;

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

const Map<String, ImmType> gSymToImmType {
#define ENTRY(x, y, z) { #z, IMM_##y },
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
    TypedSym(Symbol sym, ImmType tr, bool is_const = false);

    Symbol sym;
    ImmType tr;
    bool is_const;
};

template<typename T, typename U>
using Pair = std::pair<T, U>;

struct Code {
    pString p_code;
    String file_name;
};

typedef Pointer<Code> pCode;

template<typename T>
class Env {
public:
    typedef Pointer<Env> pEnv;

    Env(pEnv parent = {})
        : _parent(parent) { }

    bool count_current(Symbol sym) {
        return _var_map.count(sym);
    }

    bool count(Symbol sym) {
        if(count_current(sym)) {
            return true;
        }
        if(_parent) return _parent->count(sym);
        return false;
    }

    T operator [](Symbol sym) {
        return find(sym);
    }

    T find(Symbol sym) {
        if(_var_map.count(sym)) {
            return _var_map[sym];
        }
        my_assert(_parent, "Error: empty env");
        return _parent->find(sym);
    }

    void set(Symbol sym, T i) {
        _var_map[sym] = i;
    }

private:
    pEnv _parent;
    Map<String, T> _var_map;
};

template<typename T>
class EnvWrapper
{
public:
    typedef Env<T> tEnv;
    typedef Pointer<tEnv> pEnv;

    pEnv env() {
        if(env_count()) {
            return _env.top();
        }
        return pEnv();
    }

    size_t env_count() {
        return _env.size();
    }

    void push_env() {
        _env.push(pEnv(new tEnv(env())));
    }

    void end_env() {
        _env.pop();
    }

    void clear_env() {
        while(env_count())
            end_env();
    }

private:
    Stack<pEnv> _env;
};
