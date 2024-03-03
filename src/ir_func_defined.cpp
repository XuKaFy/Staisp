#include "ir_func_defined.h"

namespace Ir {

Symbol FuncDefined::print_func() const
{
    size_t line_count = 0;
    for(size_t i=0; i<body.size(); ++i) {
        body[i]->generate_line(line_count);
    }

    String whole_function = String("define ") + gImmName[var.tr] + " @" + var.sym + "("; // functions are all global

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

    whole_function += " {\n";
    for(size_t i=0; i<body.size(); ++i) {
        whole_function += body[i]->print_block();
    }
    whole_function += "}\n";

    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(TypedSym var, Vector<TypedSym> arg_types)
{
    return pFuncDefined(new FuncDefined(var, arg_types));
}

pBlock FuncDefined::current_block() const
{
    return body.back();
}

void FuncDefined::add_block(pBlock block)
{
    body.push_back(block);
}

void FuncDefined::add_instr(pInstr ir)
{
    body.back()->add_instr(ir);
}

Symbol FuncDefined::print_impl() const
{
    return var.sym;
}

}
