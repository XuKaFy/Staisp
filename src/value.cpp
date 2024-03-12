#include "value.h"

Value::operator bool() const
{
    switch(type()) {
    case VALUE_IMM:
        return (bool) imm_value();
    case VALUE_POINTER:
        return (bool) pointer_value().v;
    case VALUE_ARRAY:
        return true;
    case VALUE_STRUCT:
        return true;
    }
    throw Exception(1, "Value", "not implemented");
}

bool Value::is_static() const
{
    switch(type()) {
    case VALUE_IMM:
        return true;
    case VALUE_POINTER:
        return false;
    case VALUE_STRUCT: {
        for(auto i : struct_value())
            if(!i.v.is_static()) return false;
        return true;
    }
    case VALUE_ARRAY: {
        for(auto i : array_value())
            if(!i->is_static()) return false;
        return true;
    }
    }
    throw Exception(1, "Value", "not implemented");
    return false;
}

pValue make_value(Value v)
{
    return pValue(new Value(v));
}

pValue make_value(ImmValue v)
{
    return pValue(new Value(v));
}

pValue make_value(PointerValue v)
{
    return pValue(new Value(v));
}

pValue make_value(ArrayValue v)
{
    return pValue(new Value(v));
}

pValue make_value(StructValue v)
{
    return pValue(new Value(v));
}
