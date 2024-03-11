#pragma once

#include "imm.h"
#include "type.h"

#include <variant>

struct Value;

typedef Vector<Value> ArrayValue;
typedef Pointer<Value> PointerValue;

struct ElemValue;

typedef Vector<ElemValue> StructValue;

enum ValueType {
    VALUE_IMM = 0,
    VALUE_POINTER = 1,
    VALUE_ARRAY = 2,
    VALUE_STRUCT = 3,
};

struct Value {
    Value()
        : val() { }
    Value(ImmValue v)
        : val(v) { }
    Value(PointerValue v)
        : val(v) { }
    Value(ArrayValue v)
        : val(v) { }
    Value(StructValue v)
        : val(v) { }

    ValueType type() const {
        return (ValueType) val.index();
    }

    operator bool() const;

    ImmValue& imm_value() { return std::get<ImmValue>(val); }
    PointerValue& pointer_value() { return std::get<PointerValue>(val); }
    ArrayValue& array_value() { return std::get<ArrayValue>(val); }
    StructValue& struct_value() { return std::get<StructValue>(val); }
    
    const ImmValue& imm_value() const { return std::get<ImmValue>(val); }
    const PointerValue& pointer_value() const { return std::get<PointerValue>(val); }
    const ArrayValue& array_value() const { return std::get<ArrayValue>(val); }
    const StructValue& struct_value() const { return std::get<StructValue>(val); }

private:
    std::variant<ImmValue, PointerValue, ArrayValue, StructValue> val;
};

typedef Opt<Value> ValueOrVoid;
typedef Vector<Value> Values;

struct ElemValue {
    Symbol name;
    pType ty;
    Value v;
};
