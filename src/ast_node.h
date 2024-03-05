#pragma once

#include "def.h"
#include "common_node.h"
#include "common_token.h"

namespace Ast {

struct ImmNode : public Node
{
    ImmNode(pToken t, Immediate imm, ImmType tr) 
        : Node(t, NODE_IMM), imm(imm), tr(tr) { }
    Immediate imm;
    ImmType tr;
};

struct SymNode : public Node
{
    SymNode(pToken t,Symbol sym) : Node(t, NODE_SYM), sym(sym) { }
    Symbol sym;
};

struct OprNode : public Node
{
    OprNode(pToken t,OprType type, const AstProg &ch)
        : Node(t, NODE_OPR), type(type), ch(ch) { }
    OprType type;
    AstProg ch;
};

struct AssignNode : public Node
{
    AssignNode(pToken t,Symbol sym, pNode val)
        : Node(t, NODE_ASSIGN), sym(sym), val(val) { }
    Symbol sym;
    pNode val;
};

struct VarDefNode : public Node
{
    VarDefNode(pToken t,TypedSym var, pNode val)
        : Node(t, NODE_DEF_VAR), var(var), val(val) { }
    TypedSym var;
    pNode val;
};

struct FuncDefNode : public Node
{
    FuncDefNode(pToken t,TypedSym var, Vector<TypedSym> args, pNode body)
        : Node(t, NODE_DEF_FUNC), var(var), args(args), body(body) { }
    TypedSym var;
    Vector<TypedSym> args;
    pNode body;
};

struct BlockNode : public Node
{
    BlockNode(pToken t, AstProg body)
        : Node(t, NODE_BLOCK), body(body) { }
    AstProg body;
};

struct ArrayDefNode : public Node
{
    ArrayDefNode(pToken t, Immediates nums)
        : Node(t, NODE_BLOCK), nums(nums) { }
    Immediates nums;
};

pNode new_imm_node(pToken t, Immediate imm, ImmType tr = IMM_I64);
pNode new_sym_node(pToken t, Symbol symbol);
pNode new_opr_node(pToken t, OprType type, AstProg ch);
pNode new_assign_node(pToken t, Symbol sym, pNode val);
pNode new_var_def_node(pToken t, TypedSym var, pNode val);
pNode new_func_def_node(pToken t, TypedSym var, Vector<TypedSym> args, pNode body);
pNode new_block_node(pToken t, AstProg body);
pNode new_array_def_node(pToken t, Immediates nums);

} // namespace ast
