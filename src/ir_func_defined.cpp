#include "ir_func_defined.h"

namespace Ir {

Symbol FuncDefined::print_impl() const
{
    String whole_function = String("define ") + gImmName[tr] + " " + func_name->print() + "(";

    my_assert(!body.empty(), "Error: function has no block");

    if(args.empty()) goto PRINT_IMPL_END;
    
    whole_function += gImmName[args[0].tr];
    whole_function += " %";
    whole_function += args[0].sym;
    
    for(size_t i=1; i<args.size(); ++i) {
        whole_function += ", ";
        whole_function += gImmName[args[i].tr];
        whole_function += " %";
        whole_function += args[i].sym;
    }

PRINT_IMPL_END:
    whole_function += ")";
    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(ImmType tr, pSym func_name, Vector<TypedSym> arg_types)
{
    return pFuncDefined(new FuncDefined(tr, func_name, arg_types));
}

void FuncDefined::add_block(pBlock block)
{
    body.push_back(block);
}

Symbol FuncDefined::print_func() const
{
    String ans = print_impl();
    ans += " {\n";
    int line_count = 0;
    for(size_t i=0; i<body.size(); ++i) {
        ans += body[i]->print_block(line_count);
        line_count = body[i]->instrs.back()->line + 1;
    }
    ans += "}\n";
    return to_symbol(ans);
}

}
