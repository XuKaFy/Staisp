#include "value.h"

#include <memory>

#include "def.h"
#include "type.h"
#include <utility>

Value::operator bool() const {
    switch (type()) {
    case VALUE_IMM:
        return (bool)imm_value();
    case VALUE_ARRAY:
        return true;
    }
    throw Exception(1, "Value", "not implemented");
}

bool Value::is_static() const {
    switch (type()) {
    case VALUE_IMM:
        return true;
    case VALUE_ARRAY: {
        for (const auto &i : array_value().values) {
            if (!i->is_static()) {
                return false;
            }
        }
        return true;
    }
    }
    throw Exception(1, "Value", "not implemented");
    return false;
}

String Value::to_string() const {
    if (ty->type_type() == TYPE_VOID_TYPE) {
        return "zeroinitializer";
    }
    switch (type()) {
    case VALUE_IMM:
        return imm_value().print();
    case VALUE_ARRAY: {
        return "zeroinitializer";
/* depreciated
        String s = " [";
        bool first = true;
        for (const auto &i : array_value().values) {
            if (first) {
                first = false;
            } else {
                s += ", ";
            }
            s += array_value().ty->type_name();
            s += " ";
            s += i->to_string();
        }
        s += "]";
        return s;
*/
    }
    default:
        throw Exception(1, "Value", "not implemented");
    }
    return "";
}

pValue make_value(Value v) { return std::make_shared<Value>(std::move(v)); }

pValue make_value(ImmValue v) { return std::make_shared<Value>(v); }

pValue make_value(ArrayValue v) {
    return std::make_shared<Value>(std::move(v));
}

bool operator==(const ArrayValue &lhs, const ArrayValue &rhs) {
    return lhs.values == rhs.values && is_same_type(lhs.ty, rhs.ty);
}

size_t ArrayValue::hash() const {
    std::hash<Value> hasher;
    size_t h = 0;
    for (auto&& value : this->values) {
        h <<= 1;
        h ^= hasher(*value);
    }
    return h;
}
