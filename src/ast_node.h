#pragma once

#include "def.h"

namespace Ast {

enum NodeType
{
    NODE_IMM,
    NODE_OPR,
    NODE_SYM,
    NODE_ASSIGN,
    NODE_DEF_VAR,
    NODE_DEF_FUNC,
    NODE_BLOCK,
};

enum OprType
{
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_SDIV,
    OPR_REM,
    OPR_AND,
    OPR_OR,
    OPR_EQ,
    OPR_NE,
    OPR_UGT,
    OPR_UGE,
    OPR_ULT,
    OPR_ULE,
    OPR_SGT,
    OPR_SGE,
    OPR_SLT,
    OPR_SLE,
    OPR_IF,
    OPR_RET,
    OPR_CALL,
    OPR_WHILE,
};

struct Node
{
    Node(NodeType type)
        : type(type) { }
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef Vector<pNode> AstProg;

struct ImmNode : public Node
{
    ImmNode(Immediate imm, ImmType tr) 
        : Node(NODE_IMM), imm(imm), tr(tr) { }
    Immediate imm;
    ImmType tr;
};

struct SymNode : public Node
{
    SymNode(Symbol sym) : Node(NODE_SYM), sym(sym) { }
    Symbol sym;
};

struct OprNode : public Node
{
    OprNode(OprType type, const Vector<pNode> &ch)
        : Node(NODE_OPR), type(type), ch(ch) { }
    OprType type;
    Vector<pNode> ch;
};

struct AssignNode : public Node
{
    AssignNode(Symbol sym, pNode val)
        : Node(NODE_ASSIGN), sym(sym), val(val) { }
    Symbol sym;
    pNode val;
};

struct VarDefNode : public Node
{
    VarDefNode(TypedSym var, pNode val)
        : Node(NODE_DEF_VAR), var(var), val(val) { }
    TypedSym var;
    pNode val;
};

struct FuncDefNode : public Node
{
    FuncDefNode(TypedSym var, Vector<TypedSym> args, pNode body)
        : Node(NODE_DEF_FUNC), var(var), args(args), body(body) { }
    TypedSym var;
    Vector<TypedSym> args;
    pNode body;
};

struct BlockNode : public Node
{
    BlockNode(Vector<pNode> body)
        : Node(NODE_BLOCK), body(body) { }
    Vector<pNode> body;
};

pNode newImmNode(Immediate imm, ImmType tr = IMM_I64);
pNode newSymNode(Symbol symbol);
pNode newOprNode(OprType type, Vector<pNode> ch);
pNode newAssignNode(Symbol sym, pNode val);
pNode newVarDefNode(TypedSym var, pNode val);
pNode newFuncDefNode(TypedSym var, Vector<TypedSym> args, pNode body);
pNode newBlockNode(Vector<pNode> body);

} // namespace ast
