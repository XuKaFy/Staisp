#include "ir_func.h"

namespace Ir {

Symbol Func::print_impl() const
{
    return func_name->print();
}

pFunc make_func(ImmType tr, pSym func_name, Vector<ImmType> arg_types)
{
    return pFunc(new Func(tr, func_name, arg_types));
}

}
