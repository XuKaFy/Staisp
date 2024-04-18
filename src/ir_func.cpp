#include "ir_func.h"

namespace Ir {

pFunctionType Func::functon_type() const
{
    return to_function_type(ty);
}

pFunc make_func(TypedSym var, Vector<pType> arg_types)
{
    return pFunc(new Func(var, arg_types));
}

}
