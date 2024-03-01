#include "ast_node.h"

namespace Ast {

pNode newImmNode(Immediate imm, int bits)
{
    return pNode(new ImmNode(imm, bits));
}

pNode newSymNode(Symbol str)
{
    return pNode(new SymNode(str));
}

pNode newOprNode(Vector<pNode> ch, OprType type)
{
    return pNode(new OprNode(ch, type));
}

}  // namespace ast
