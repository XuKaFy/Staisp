#pragma once

#include "imm.h"
#include "type.h"

#include <variant>

struct Value;
using pValue = Pointer<Value>;

struct ArrayValue {
    Vector<pValue> values;
    pType ty;

    friend bool operator==(const ArrayValue& lhs, const ArrayValue& rhs);
    size_t hash() const;
};

struct ElemValue;

enum ValueType {
    VALUE_IMM,
    VALUE_ARRAY,
};

struct Value {
    Value() : ty(make_void_type()) {}
    Value(ImmValue v) : val(v), ty(make_basic_type(v.ty)) {}
    Value(ArrayValue v) : val(v), ty(make_array_type(v.ty, v.values.size())) {}

    void reset_value(const Value &v) { val = v.val; }

    ValueType type() const { return static_cast<ValueType>(val.index()); }

    explicit operator bool() const;

    String to_string() const;

    ImmValue &imm_value() { return std::get<ImmValue>(val); }
    ArrayValue &array_value() { return std::get<ArrayValue>(val); }

    const ImmValue &imm_value() const { return std::get<ImmValue>(val); }
    const ArrayValue &array_value() const { return std::get<ArrayValue>(val); }

    bool is_static() const;

    std::variant<ImmValue, ArrayValue> val;
    pType ty;

    friend bool operator==(const Value& lhs, const Value& rhs) {
        return lhs.val == rhs.val && is_same_type(lhs.ty, rhs.ty);
    }
};

template<typename... Ts>
struct overloaded : Ts... {
    explicit overloaded(Ts... ts): Ts(ts)... {}
    using Ts::operator()...;
};


namespace std {

template<>
struct hash<Value> {
    size_t operator()(const Value& value) const noexcept {
        return std::hash<std::string>()(value.ty->type_name()) ^ std::visit(overloaded {
            [](const ImmValue& value) -> size_t { return value.hash(); },
            [](const ArrayValue& value) -> size_t { return value.hash(); },
        }, value.val);
    }
};

}


pValue make_value(Value v);
pValue make_value(ImmValue v);
pValue make_value(ArrayValue v);
