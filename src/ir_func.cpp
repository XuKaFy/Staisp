#include "ir_func.h"

namespace Ir {

Symbol Func::print_impl() const
{
    // functions are all global
    return to_symbol(String("@") + var.sym);
}

pFunc make_func(ImmTypedSym var, Vector<ImmTypedSym> arg_types)
{
    return pFunc(new Func(var, arg_types));
}

}
