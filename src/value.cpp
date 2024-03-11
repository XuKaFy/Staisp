#include "value.h"

Value::operator bool() const
{
    switch(type()) {
    case VALUE_IMM:
        return (bool) imm_value();
    case VALUE_POINTER:
        return (bool) pointer_value();
    case VALUE_ARRAY:
        return true;
    case VALUE_STRUCT:
        return true;
    }
    throw Exception(1, "Value", "not implemented");
}
