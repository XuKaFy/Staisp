// 将常用的对于嵌套作用域的操作抽象而成的类
// 用于 Ast 转 IR 或者 Parser 的变量存在检查

#pragma once

#include "def.h"

// Env<T> 即
// 在某一层作用域内，保存一个 String -> T 的映射
// 所有的查找都将在本层和上层（若本层未找到）进行
// 但修改只会在本层
template<typename T>
class Env {
public:
    typedef Pointer<Env> pEnv;

    Env(pEnv parent = pEnv())
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

    T& operator [](Symbol sym) {
        return find(sym);
    }

    T& find(Symbol sym) {
        if(_var_map.count(sym)) {
            return _var_map[sym];
        }
        if(_parent)
            return _parent->find(sym);
        throw Exception(1, "Env", "cannot find variant in this environment");
    }

    void set(Symbol sym, T i) {
        _var_map[sym] = i;
    }

private:
    pEnv _parent;
    Map<String, T> _var_map;
};

// EnvWrapper<T> 即每层都记录 String -> T 的映射的多层作用域
// 内部用栈实现，使用 push_env 和 end_env 模拟作用域进入和退出的过程
// push_env 时，新创建的作用域将会自动以上一层的作用域作为父节点
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
        throw Exception(1, "EnvWrapper", "current environment is empty");
    }

    size_t env_count() {
        return _env.size();
    }

    void push_env() {
        if(env_count())
            _env.push(pEnv(new tEnv(env())));
        else
            _env.push(pEnv(new tEnv()));
    }

    void end_env() {
        if(_env.empty())
            throw Exception(2, "EnvWrapper", "environment stack is empty");
        _env.pop();
    }

    void clear_env() {
        while(env_count())
            end_env();
    }

private:
    Stack<pEnv> _env;
};

