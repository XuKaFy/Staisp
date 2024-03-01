#include "node.h"

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

