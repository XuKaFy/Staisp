#include "ir_func_defined.h"

namespace Ir {

FuncDefined::FuncDefined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> arg_name)
    : Func(var, arg_types) {
    this->arg_name = arg_name;
    size_t length = arg_types.size();
    add_body(make_label_instr());
    for(size_t i=0; i<length; ++i) {
        auto sym_node = make_sym_instr(TypedSym(to_symbol(String("%") + arg_name[i]), arg_types[i]));
        pInstr alloced = make_alloc_instr(arg_types[i]);
        pInstr stored = make_store_instr(alloced, sym_node);
        add_body(alloced);
        add_body(stored);
        add_imm(sym_node);
        args.push_back(alloced);
    }
}

void FuncDefined::add_body(pInstr instr)
{
    body.push_back(instr);
}

void FuncDefined::add_imm(pVal val)
{
    imms.push_back(val);
}

void FuncDefined::end_function()
{
    p.from_instrs(body);
}

Symbol FuncDefined::print_func() const
{
    String whole_function = String("define ") 
        + ty->type_name() + " @" + name() + "("; // functions are all global

    my_assert(!p.blocks.empty(), "Error: function has no block");
    
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

    whole_function += " {\n";
    for(auto i : p.blocks) {
        whole_function += i->print_block();
    }
    whole_function += "}\n";

    return to_symbol(whole_function);
}

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> syms)
{
    return pFuncDefined(new FuncDefined(var, arg_types, syms));
}

}
