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

pNode newVarDefNode(Symbol sym, Immediate val, ImmType tr)
{
    return pNode(new VarDefNode(sym, val, tr));
}

pNode newFuncDefNode(Symbol sym, ImmType tr, Vector<TypedSym> args, Vector<pNode> body)
{
    return pNode(new FuncDefNode(sym, tr, args, body));
}

}  // namespace ast
