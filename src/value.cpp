#include "value.h"

#include <memory>

#include <utility>
#include "def.h"
#include "type.h"

Value::operator bool() const {
    switch (type()) {
    case VALUE_IMM:
        return (bool)imm_value();
    case VALUE_POINTER:
        return (bool)pointer_value().v;
    case VALUE_ARRAY:
        return true;
    case VALUE_STRUCT:
        return true;
    }
    throw Exception(1, "Value", "not implemented");
}

bool Value::is_static() const {
    switch (type()) {
    case VALUE_IMM:
        return true;
    case VALUE_POINTER:
        return false;
    case VALUE_STRUCT: {
        for (const auto& i : struct_value()) {
            if (!i.v.is_static()) {
                return false;
}
}
        return true;
    }
    case VALUE_ARRAY: {
        for (const auto& i : array_value().arr) {
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
    /*case VALUE_POINTER:
        return pointer_value()();
    case VALUE_STRUCT: {
        for(auto i : struct_value())
            if(!i.v.is_static()) return false;
        return true;
    }
    }*/
    case VALUE_ARRAY: {
        String s = " [";
        bool first = true;
        for (const auto& i : array_value().arr) {
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
    }
    default:
        throw Exception(1, "Value", "not implemented");
    }
    return "";
}

pValue make_value(Value v) { return std::make_shared<Value>(std::move(v)); }

pValue make_value(ImmValue v) { return std::make_shared<Value>(v); }

pValue make_value(PointerValue v) { return std::make_shared<Value>(std::move(v)); }

pValue make_value(ArrayValue v) { return std::make_shared<Value>(std::move(v)); }

/*
pValue make_value(StructValue v)
{
    return pValue(new Value(v));
}
*/
