#include "ir_func_defined.h"

namespace Ir {

Symbol FuncDefined::print_impl() const
{
    String whole_function = String("define ") + gImmName[tr] + " @" + func_name;
    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(ImmType tr, Symbol func_name, Vector<ImmType> arg_types)
{
    return pFunc(new FuncDefined(tr, func_name, arg_types));
}

}
