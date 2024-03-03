#include "ast_node.h"

namespace Ast {

pNode newImmNode(Immediate imm, ImmType tr)
{
    return pNode(new ImmNode(imm, tr));
}

pNode newSymNode(Symbol str)
{
    return pNode(new SymNode(str));
}

pNode newOprNode(OprType type, Ast::AstProg ch)
{
    return pNode(new OprNode(type, ch));
}

pNode newAssignNode(Symbol sym, pNode val)
{
    return pNode(new AssignNode(sym, val));
}

pNode newVarDefNode(TypedSym var, pNode val)
{
    return pNode(new VarDefNode(var, val));
}

pNode newFuncDefNode(TypedSym var, Vector<TypedSym> args, pNode body)
{
    return pNode(new FuncDefNode(var, args, body));
}

pNode newBlockNode(Ast::AstProg body)
{
    return pNode(new BlockNode(body));
}

}  // namespace ast
