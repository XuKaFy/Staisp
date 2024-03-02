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
};

enum OprType
{
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_DIV,
    OPR_REM,
    OPR_AND,
    OPR_OR,
    OPR_EQ,
    OPR_IF,
    OPR_RET,
    OPR_CALL,
    OPR_WHILE,
};

struct Node
{
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef Vector<pNode> AstProg;

struct ImmNode : public Node
{
    ImmNode(Immediate imm, ImmType tr) 
        : imm(imm), tr(tr) { }
    Immediate imm;
    ImmType tr;
};

struct SymNode : public Node
{
    SymNode(Symbol sym) : sym(sym) { }
    Symbol sym;
};

struct OprNode : public Node
{
    OprNode(OprType type, const Vector<pNode> &ch)
        : type(type), ch(ch) { }
    OprType type;
    Vector<pNode> ch;
};

struct AssignNode : public Node
{
    AssignNode(Symbol sym, pNode val)
        : sym(sym), val(val) { }
    Symbol sym;
    pNode val;
};

struct VarDefNode : public Node
{
    VarDefNode(Symbol sym, ImmType tr)
        : sym(sym), tr(tr) { }
    Symbol sym;
    ImmType tr;
};

struct TypedSym
{
    Symbol sym;
    ImmType tr;
};

struct FuncDefNode : public Node
{
    FuncDefNode(Symbol sym, ImmType tr, Vector<TypedSym> args, Vector<pNode> body)
        : sym(sym), tr(tr), args(args), body(body) { }
    Symbol sym;
    ImmType tr;
    Vector<TypedSym> args;
    Vector<pNode> body;
};

pNode newImmNode(Immediate imm, ImmType tr = IMM_I64);
pNode newSymNode(Symbol symbol);
pNode newOprNode(OprType type, Vector<pNode> ch);
pNode newAssignNode(Symbol sym, pNode val);
pNode newVarDefNode(Symbol sym, ImmType tr);
pNode newFuncDefNode(Symbol sym, ImmType tr, Vector<TypedSym> args, Vector<pNode> body);

} // namespace ast
