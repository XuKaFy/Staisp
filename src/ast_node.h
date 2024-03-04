#pragma once

#include "def.h"
#include "staisp_parser.h"

namespace Ast {

struct ImmNode : public Node
{
    ImmNode(Staisp::Token t, Immediate imm, ImmType tr) 
        : Node(t, NODE_IMM), imm(imm), tr(tr) { }
    Immediate imm;
    ImmType tr;
};

struct SymNode : public Node
{
    SymNode(Staisp::Token t,Symbol sym) : Node(t, NODE_SYM), sym(sym) { }
    Symbol sym;
};

struct OprNode : public Node
{
    OprNode(Staisp::Token t,OprType type, const Ast::AstProg &ch)
        : Node(t, NODE_OPR), type(type), ch(ch) { }
    OprType type;
    Ast::AstProg ch;
};

struct AssignNode : public Node
{
    AssignNode(Staisp::Token t,Symbol sym, pNode val)
        : Node(t, NODE_ASSIGN), sym(sym), val(val) { }
    Symbol sym;
    pNode val;
};

struct VarDefNode : public Node
{
    VarDefNode(Staisp::Token t,TypedSym var, pNode val)
        : Node(t, NODE_DEF_VAR), var(var), val(val) { }
    TypedSym var;
    pNode val;
};

struct FuncDefNode : public Node
{
    FuncDefNode(Staisp::Token t,TypedSym var, Vector<TypedSym> args, pNode body)
        : Node(t, NODE_DEF_FUNC), var(var), args(args), body(body) { }
    TypedSym var;
    Vector<TypedSym> args;
    pNode body;
};

struct BlockNode : public Node
{
    BlockNode(Staisp::Token t, Ast::AstProg body)
        : Node(t, NODE_BLOCK), body(body) { }
    Ast::AstProg body;
};

pNode new_imm_node(Staisp::Token t, Immediate imm, ImmType tr = IMM_I64);
pNode new_sym_node(Staisp::Token t, Symbol symbol);
pNode new_opr_node(Staisp::Token t, OprType type, Ast::AstProg ch);
pNode new_assign_node(Staisp::Token t, Symbol sym, pNode val);
pNode new_var_def_node(Staisp::Token t, TypedSym var, pNode val);
pNode new_func_def_node(Staisp::Token t, TypedSym var, Vector<TypedSym> args, pNode body);
pNode new_block_node(Staisp::Token t, Ast::AstProg body);

} // namespace ast
