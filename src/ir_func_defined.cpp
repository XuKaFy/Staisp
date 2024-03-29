#include "ir_func_defined.h"

namespace Ir {

FuncDefined::FuncDefined(TypedSym var, Vector<TypedSym> arg_types)
    : Func(var, arg_types) {
    pBlock first_block = make_block();
    
    for(auto i : arg_types) {
        if(is_const_type(i.tr)) {
            args_value.push_back(make_const_arg(i));
        } else {
            pInstr tmp = first_block->add_instr(make_alloc_instr(i.tr));
            first_block->add_instr(make_store_instr(tmp, make_sym_instr(i)));
            args_value.push_back(tmp);
        }
    }
    
    body.push_back(first_block);
}

Symbol FuncDefined::print_func() const
{
    size_t line_count = 0;
    for(size_t i=0; i<body.size(); ++i) {
        body[i]->generate_line(line_count);
    }

    String whole_function = String("define ") 
        + var.tr->type_name() + " @" + var.sym + "("; // functions are all global

    my_assert(!body.empty(), "Error: function has no block");

    if(args.empty()) goto PRINT_IMPL_END;
    
    whole_function += args[0].tr->type_name();
    whole_function += " %";
    whole_function += args[0].sym;
    
    for(size_t i=1; i<args.size(); ++i) {
        whole_function += ", ";
        whole_function += args[i].tr->type_name();
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

void FuncDefined::shrink()
{
    Vector<pBlock> n;
    for(auto i : body)
        if(i->instrs.size() > 1)
            n.push_back(i);
    body = n;
}

pBlock FuncDefined::current_block() const
{
    return body.back();
}

void FuncDefined::add_block(pBlock block)
{
    body.push_back(block);
}

pInstr FuncDefined::add_instr(pInstr ir)
{
    return body.back()->add_instr(ir);
}

Symbol FuncDefined::print_impl() const
{
    return var.sym;
}

}
