#pragma once

#include "imm.h"
#include "type.h"

#include <variant>

struct Value;
using pValue = Pointer<Value>;

struct PointerValue {
    pValue v;
};

struct ArrayValue {
    Vector<pValue> arr;
    pType ty;
};

struct ElemValue;

using StructValue = Vector<ElemValue>;

enum ValueType {
    VALUE_IMM = 0,
    VALUE_POINTER = 1,
    VALUE_ARRAY = 2,
    VALUE_STRUCT = 3,
};

struct Value {
    Value() : ty(make_void_type()) {}
    Value(ImmValue v) : val(v), ty(make_basic_type(v.ty)) {}
    Value(PointerValue v) : val(v), ty(make_pointer_type(v.v->ty)) {}
    Value(ArrayValue v) : val(v), ty(make_array_type(v.ty, v.arr.size())) {}
    // Value(StructValue v)
    //     : val(v), ty(make_struct_type(v.v->ty)) { }

    void reset_value(const Value &v) { val = v.val; }

    ValueType type() const { return static_cast<ValueType>(val.index()); }

    operator bool() const;

    String to_string() const;

    ImmValue &imm_value() { return std::get<ImmValue>(val); }
    PointerValue &pointer_value() { return std::get<PointerValue>(val); }
    ArrayValue &array_value() { return std::get<ArrayValue>(val); }
    StructValue &struct_value() { return std::get<StructValue>(val); }

    const ImmValue &imm_value() const { return std::get<ImmValue>(val); }
    const PointerValue &pointer_value() const {
        return std::get<PointerValue>(val);
    }
    const ArrayValue &array_value() const { return std::get<ArrayValue>(val); }
    const StructValue &struct_value() const {
        return std::get<StructValue>(val);
    }

    bool is_static() const;

    std::variant<ImmValue, PointerValue, ArrayValue, StructValue> val;
    pType ty;
};

using LValueOrVoid = Opt<pValue>;
using Values = Vector<pValue>;

pValue make_value(Value v);
pValue make_value(ImmValue v);
pValue make_value(PointerValue v);
pValue make_value(ArrayValue v);
pValue make_value(StructValue v);

struct ElemValue {
    String name;
    pType ty;
    Value v;
};
