#include "convert_ast_to_ir.h"

namespace AstToIr {

Ir::pInstr Convertor::find_value(Symbol sym, Ir::pModule mod)
{
    if(var_map.count(sym)) { // local
        return var_map[sym];
    } else { // global
        // TODO: no instr can modify global vars
        for(auto i : mod->globs) {
            if(strcmp(i->name->name, sym) == 0) {
                printf("found %s: %d\n", i->name->name, i->tr);
                return Ir::make_sym_instr(TypedSym { i->name->name, i->tr }, Ir::SYM_GLOBAL);
            }
        }
    }
    my_assert(false, "Error: symbol cannot be found");
}

Ir::pInstr Convertor::analyze_value(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_OPR: {
        auto res = Ir::make_binary_instr(Ir::INSTR_ADD, IMM_I16, Ir::make_constant(-1), Ir::make_constant(-1));
        func->body.back()->instrs.push_back(res);
        return res;
    }
    case Ast::NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_binary_instr(Ir::INSTR_ADD, r->tr, Ir::make_constant(0), Ir::make_constant(r->imm));
        func->body.back()->instrs.push_back(res);
        return res;
    }
    case Ast::NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        auto to = find_value(r->sym, mod);
        return to;
    }
    case Ast::NODE_ASSIGN:
    case Ast::NODE_DEF_VAR:
    case Ast::NODE_DEF_FUNC:
        my_assert(false, "Error: not calculatable");
    }
}

void Convertor::analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_value(r->sym, mod);
        func->body.back()->instrs.push_back(Ir::make_store_instr(to->tr, to, analyze_value(r->val, func, mod)));
        break;
    }
    case Ast::NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        func->body.back()->instrs.push_back(tmp = Ir::make_alloc_instr(r->tr));
        func->body.back()->instrs.push_back(Ir::make_store_instr(r->tr, tmp, Ir::make_constant(r->val)));
        var_map[r->sym] = tmp;
        break;
    }
    case Ast::NODE_OPR:
        analyze_value(root, func, mod);
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
        func = Ir::make_func_defined(root->tr, Ir::make_sym(root->sym), root->args);
        for(size_t i=0; i<root->args.size(); ++i) {
            var_map[root->args[i].sym] = func->body[0]->instrs[i];
        }
    }
    for(auto i : root->body) {
        analyze_statement_node(i, func, mod);
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

