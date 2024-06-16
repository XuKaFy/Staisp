#pragma once

#include "type.h"

#include "ir_val.h"
#include "value.h"

namespace Ir {

struct Const : public Val {
    Const(Value value) : Val(value.ty), v(std::move(value)) { set_name(v.to_string()); }

    ValType type() const override { return VAL_CONST; }

    // Yaossg: I hate this name, but dare not to refactor
    Value v;
};

using pConst = Pointer<Const>;

struct ConstPool {
    std::unordered_map<Value, pVal> pool;

    pVal add(Value value) {
        if (auto it = pool.find(value); it != pool.end()) {
            return it->second;
        }
        pVal val{new Const(value)};
        pool.emplace(value, val);
        return val;
    }

    void clear() {
        pool.clear();
    }

    void merge(ConstPool& other) {
        pool.merge(other.pool);
        for (auto && [value, val] : other.pool) {
            val->replace_self(pool[value].get());
        }
    }
};


} // namespace Ir
