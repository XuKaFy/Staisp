#pragma once

#include "def.h"

namespace Ast {

enum NodeType
{
    NODE_IMM,
    NODE_OPR,
    NODE_SYM,
};

enum OprType
{
    OPR_ASSIGN,
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_DIV,
    OPR_REM,
};

struct Node
{
    NodeType type;
};

typedef Pointer<Node> pNode;

struct ImmNode : public Node
{
    ImmNode(Immediate imm, int bits) : imm(imm), bits(bits) { }
    Immediate imm;
    int bits;
};

struct SymNode : public Node
{
    SymNode(Symbol sym) : sym(sym) { }
    Symbol sym;
};

struct OprNode : public Node
{
    OprNode(const Vector<pNode> &ch, OprType type)
        : ch(ch), type(type) { }
    Vector<pNode> ch;
    OprType type;
};

pNode newImmNode(Immediate imm, int bits = 64);
pNode newSymNode(Symbol symbol);
pNode newOprNode(Vector<pNode> ch, OprType type);

} // namespace ast
