#pragma once

#include "def.h"

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

