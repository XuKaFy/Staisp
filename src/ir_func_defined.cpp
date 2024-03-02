#include "ir_func_defined.h"

namespace Ir {

Symbol FuncDefined::print_impl() const
{
    String whole_function = String("define ") + gImmName[tr] + " " + func_name->print() + "(";

    my_assert(!body.empty(), "Error: function has no block");
    pBlock first_block = body.front();
    my_assert(first_block->instrs.size() == arg_types.size(), "Error, Function's first block has different size from arg_types");

    if(arg_types.empty()) goto PRINT_IMPL_END;
    
    whole_function += gImmName[arg_types[0]];
    whole_function += " ";
    whole_function += first_block->instrs[0]->print();
    
    for(size_t i=1; i<first_block->instrs.size(); ++i) {
        whole_function += ", ";
        whole_function += gImmName[arg_types[i]];
        whole_function += " ";
        whole_function += first_block->instrs[i]->print();
    }

PRINT_IMPL_END:
    whole_function += ")";
    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(ImmType tr, pSym func_name, Vector<ImmType> arg_types)
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
    int line_count = body[0]->instrs.back()->line + 1;
    for(size_t i=1; i<body.size(); ++i) {
        ans += body[i]->print_block(line_count);
        line_count = body[i]->instrs.back()->line + 1;
    }
    ans += "}\n";
    return to_symbol(ans);
}

}
