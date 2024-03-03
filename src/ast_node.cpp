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

pNode newOprNode(OprType type, Vector<pNode> ch)
{
    return pNode(new OprNode(type, ch));
}

pNode newAssignNode(Symbol sym, pNode val)
{
    return pNode(new AssignNode(sym, val));
}

pNode newVarDefNode(TypedSym var, Immediate val)
{
    return pNode(new VarDefNode(var, val));
}

pNode newFuncDefNode(TypedSym var, Vector<TypedSym> args, Vector<pNode> body)
{
    return pNode(new FuncDefNode(var, args, body));
}

}  // namespace ast
