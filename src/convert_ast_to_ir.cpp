#include "convert_ast_to_ir.h"

namespace AstToIr {

void generate_single(Ast::pNode root, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_IMM:
        break;
    case Ast::NODE_OPR:
        break;
    case Ast::NODE_SYM:
        break;
    case Ast::NODE_ASSIGN:
        break;
    case Ast::NODE_DEF_FUNC:
        break;
    case Ast::NODE_DEF_VAR:
        break;
    }
}

Ir::pModule generate(Ast::AstProg asts)
{
    Ir::pModule mod = Ir::pModule(new Ir::Module());
    for(auto i : asts) {
        generate_single(i, mod);
    }
    return mod;
}

} // namespace ast_to_ir

