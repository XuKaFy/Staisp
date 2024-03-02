#include "convert_ast_to_ir.h"

namespace AstToIr {

void Convertor::analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func)
{
    switch(root->type) {
    case Ast::NODE_ASSIGN:
        break;
    case Ast::NODE_DEF_VAR:
        break;
    case Ast::NODE_OPR: // may have meaning, for executing may contain function call
        break;
    case Ast::NODE_IMM: // meaningless
    case Ast::NODE_SYM: // meaningless
        break;
    case Ast::NODE_DEF_FUNC:
        my_assert(false, "Error: function in function");
    }
}

void Convertor::generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod)
{
    var_map.clear();
    Ir::pFuncDefined func;
    {
        Vector<ImmType> no_type_args;
        int count = 0;
        for(auto i : root->args) {
            no_type_args.push_back(i.tr);
            var_map[i.sym] = count++;
        }
        func = Ir::make_func_defined(root->tr, Ir::make_sym(root->sym), no_type_args);
    }
    for(auto i : root->body) {
        analyze_statement_node(i, func);
    }
    mod->add_func(func);
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod)
{
    mod->add_global(Ir::make_global(root->tr, root->val, Ir::make_sym(root->sym)));
}

void Convertor::generate_single(Ast::pNode root, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_DEF_FUNC:
        generate_function(std::static_pointer_cast<Ast::FuncDefNode>(root), mod);
        break;
    case Ast::NODE_DEF_VAR:
        generate_global_var(std::static_pointer_cast<Ast::VarDefNode>(root), mod);
        break;
    default:
        my_assert(false, "Error: global node has type that not implemented");
    }
}

Ir::pModule Convertor::generate(Ast::AstProg asts)
{
    Ir::pModule mod = Ir::pModule(new Ir::Module());
    for(auto i : asts) {
        generate_single(i, mod);
    }
    return mod;
}

} // namespace ast_to_ir

