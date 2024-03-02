#include "ir_func.h"

namespace Ir {

Symbol Func::print_impl() const
{
    return to_symbol(String("@") + func_name);
}

pFunc make_func(ImmType tr, Symbol func_name, Vector<ImmType> arg_types)
{
    return pFunc(new Func(tr, func_name, arg_types));
}

}
