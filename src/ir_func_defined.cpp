#include "ir_func_defined.h"

namespace Ir {

FuncDefined::FuncDefined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> arg_name)
    : Func(var, arg_types) {
    this->arg_name = arg_name;
    size_t length = arg_types.size();
    add_body(make_label_instr());
    for(size_t i=0; i<length; ++i) {
        pInstr alloced = make_alloc_instr(arg_types[i]);
        pInstr stored = make_store_instr(alloced, make_sym_instr(TypedSym(to_symbol(String("%") + arg_name[i]), arg_types[i])));
        add_body(alloced);
        add_body(stored);
        args.push_back(alloced);
    }
}

void FuncDefined::add_body(pInstr instr)
{
    body.push_back(instr);
}

Symbol FuncDefined::print_func() const
{
    String whole_function = String("define ") 
        + ty->type_name() + " @" + name() + "("; // functions are all global

    my_assert(!body.empty(), "Error: function has no block");
    
    auto func_ty = functon_type();

    if(args.empty()) goto PRINT_IMPL_END;

    whole_function += func_ty->arg_type[0]->type_name();
    whole_function += " %";
    whole_function += arg_name[0];
    
    for(size_t i=1; i<args.size(); ++i) {
        whole_function += ", ";
        whole_function += func_ty->arg_type[i]->type_name();
        whole_function += " %";
        whole_function += arg_name[i];
    }

PRINT_IMPL_END:
    whole_function += ")";

    LineGenerator g;

    whole_function += " {\n";
    for(size_t i=0; i<body.size(); ++i) {
        if(!body[i]->has_name()) {
            switch(body[i]->ty->type_type()) {
            case TYPE_VOID_TYPE:
            case TYPE_RETURN_TYPE:
                break;
            case TYPE_LABEL_TYPE:
                body[i]->set_name(String("L") + g.label());    
                break;
            case TYPE_BASIC_TYPE:
            case TYPE_COMPOUND_TYPE:
                body[i]->set_name(String("%") + std::to_string(g.line()));    
                break;
            }
        }
    }

    for(size_t i=0; i<body.size(); ++i) {
        whole_function += body[i]->instr_print();
        whole_function += "\n";
    }
    whole_function += "}\n";

    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> syms)
{
    return pFuncDefined(new FuncDefined(var, arg_types, syms));
}

}
