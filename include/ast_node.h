#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "common_node.h"
#include "common_token.h"

namespace Ast {

struct ImmNode : public Node
{
    ImmNode(pToken t, ImmValue imm) 
        : Node(t, NODE_IMM), imm(imm) { }
    ImmValue imm;
};

struct SymNode : public Node
{
    SymNode(pToken t, Symbol sym) : Node(t, NODE_SYM), sym(sym) { }
    Symbol sym;
};

struct BinaryNode : public Node
{
    BinaryNode(pToken t, BinaryType type, pNode lhs, pNode rhs)
        : Node(t, NODE_BINARY), type(type), lhs(lhs), rhs(rhs) { }

    BinaryType type;
    pNode lhs, rhs;
};

struct UnaryNode : Node {

    UnaryNode(pToken t, UnaryType type, pNode ch)
        : Node(t, NODE_UNARY), type(type), ch(ch) {}

    UnaryType type;
    pNode ch;
};

struct CallNode : public Node
{
    CallNode(pToken t, Symbol name, Vector<pNode> ch)
        : Node(t, NODE_CALL), name(name), ch(ch) { }
    Symbol name;
    Vector<pNode> ch;
};

struct AssignNode : public Node
{
    AssignNode(pToken t, pNode lv, pNode val)
        : Node(t, NODE_ASSIGN), lv(lv), val(val) { }
    pNode lv;
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
    FuncDefNode(pToken t, TypedSym var, Vector<TypedSym> args, pNode body)
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

struct IfNode : public Node
{
    IfNode(pToken t, pNode cond, pNode body, pNode elsed = {})
        : Node(t, NODE_IF), cond(cond), body(body), elsed(elsed) { }
    pNode cond;
    pNode body;
    pNode elsed;
};

struct WhileNode : public Node
{
    WhileNode(pToken t, pNode cond, pNode body)
        : Node(t, NODE_WHILE), cond(cond), body(body) { }
    pNode cond;
    pNode body;
};

struct ForNode : public Node
{
    ForNode(pToken t, pNode init, pNode cond, pNode exec, pNode body)
        : Node(t, NODE_FOR), init(init), cond(cond), exec(exec), body(body) { }
    pNode init;
    pNode cond;
    pNode exec;
    pNode body;
};

struct BreakNode : public Node
{
    BreakNode(pToken t)
        : Node(t, NODE_BREAK) {}
};

struct ContinueNode : public Node
{
    ContinueNode(pToken t)
        : Node(t, NODE_CONTINUE) {}
};

struct ReturnNode : public Node
{
    ReturnNode(pToken t, pNode ret = {})
        : Node(t, NODE_RETURN), ret(ret) {}
    
    pNode ret;
};

struct ArrayDefNode : public Node
{
    ArrayDefNode(pToken t, Vector<pNode> nums)
        : Node(t, NODE_ARRAY_VAL), nums(nums) { }
    Vector<pNode> nums;
};

struct CastNode : public Node
{
    CastNode(pToken t, pType ty, pNode val)
        : Node(t, NODE_CAST), ty(ty), val(val) { }
    pType ty;
    pNode val;
};

struct RefNode : public Node
{
    RefNode(pToken t, Symbol name)
        : Node(t, NODE_REF), name(name) { }
    Symbol name;
};

struct DerefNode : public Node
{
    DerefNode(pToken t, pNode val)
        : Node(t, NODE_DEREF), val(val) { }
    pNode val;
};

struct ItemNode : public Node
{
    ItemNode(pToken t, Symbol v, Vector<pNode> index)
        : Node(t, NODE_ITEM), v(v), index(index) { }
    Symbol v;
    Vector<pNode> index;
};

pNode new_imm_node(pToken t, ImmValue imm);
pNode new_sym_node(pToken t, Symbol symbol);

pNode new_block_node(pToken t, AstProg body);

pNode new_binary_node(pToken t, BinaryType type, pNode lhs, pNode rhs);
pNode new_unary_node(pToken t, UnaryType type, pNode rhs);
pNode new_cast_node(pToken t, pType ty, pNode val);
pNode new_ref_node(pToken t, Symbol name);
pNode new_deref_node(pToken t, pNode val);
pNode new_item_node(pToken t, Symbol v, Vector<pNode> index);

pNode new_var_def_node(pToken t, TypedSym var, pNode val);
pNode new_func_def_node(pToken t, TypedSym var, Vector<TypedSym> args, pNode body);
pNode new_array_def_node(pToken t, Vector<pNode> nums);

pNode new_assign_node(pToken t, pNode lv, pNode val);
pNode new_if_node(pToken t, pNode cond, pNode body, pNode elsed = {});
pNode new_while_node(pToken t, pNode cond, pNode body);
pNode new_for_node(pToken t, pNode init, pNode cond, pNode exec, pNode body);
pNode new_break_node(pToken t);
pNode new_return_node(pToken t, pNode ret = {});
pNode new_continue_node(pToken t);
pNode new_call_node(pToken t, Symbol name, Vector<pNode> ch);

} // namespace ast
