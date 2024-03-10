#include "ir_func.h"

namespace Ir {

Symbol Func::print_impl() const
{
    // functions are all global
    return var.sym;
}

pFunc make_func(TypedSym var, Vector<TypedSym> arg_types)
{
    return pFunc(new Func(var, arg_types));
}

}
