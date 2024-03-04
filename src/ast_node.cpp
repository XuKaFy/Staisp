#include "ast_node.h"

namespace Ast {

pNode new_imm_node(Staisp::Token t, Immediate imm, ImmType tr)
{
    return pNode(new ImmNode(t, imm, tr));
}

pNode new_sym_node(Staisp::Token t, Symbol str)
{
    return pNode(new SymNode(t, str));
}

pNode new_opr_node(Staisp::Token t, OprType type, Ast::AstProg ch)
{
    return pNode(new OprNode(t, type, ch));
}

pNode new_assign_node(Staisp::Token t, Symbol sym, pNode val)
{
    return pNode(new AssignNode(t, sym, val));
}

pNode new_var_def_node(Staisp::Token t, TypedSym var, pNode val)
{
    return pNode(new VarDefNode(t, var, val));
}

pNode new_func_def_node(Staisp::Token t, TypedSym var, Vector<TypedSym> args, pNode body)
{
    return pNode(new FuncDefNode(t, var, args, body));
}

pNode new_block_node(Staisp::Token t, Ast::AstProg body)
{
    return pNode(new BlockNode(t, body));
}

}  // namespace ast
