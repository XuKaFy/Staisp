#include "ast_node.h"

namespace Ast {

pNode new_imm_node(pToken t, ImmValue imm)
{
    return pNode(new ImmNode(t, imm));
}

pNode new_sym_node(pToken t, Symbol str)
{
    return pNode(new SymNode(t, str));
}

pNode new_binary_node(pToken t, BinaryType type, pNode lhs, pNode rhs)
{
    return pNode(new BinaryNode(t, type, lhs, rhs));
}

pNode new_unary_node(pToken t, UnaryType type, pNode rhs)
{
    return pNode(new UnaryNode(t, type, rhs));
}

pNode new_assign_node(pToken t, pNode lv, pNode val)
{
    return pNode(new AssignNode(t, lv, val));
}

pNode new_var_def_node(pToken t, TypedSym var, pNode val, bool is_const)
{
    return pNode(new VarDefNode(t, var, val, is_const));
}

pNode new_func_def_node(pToken t, TypedSym var, Vector<TypedSym> args, pNode body)
{
    return pNode(new FuncDefNode(t, var, args, body));
}

pNode new_block_node(pToken t, AstProg body)
{
    return pNode(new BlockNode(t, body));
}

pNode new_array_def_node(pToken t, Vector<pNode> nums)
{
    return pNode(new ArrayDefNode(t, nums));
}

pNode new_cast_node(pToken t, pType ty, pNode val)
{
    return pNode(new CastNode(t, ty, val));
}

pNode new_ref_node(pToken t, Symbol name)
{
    return pNode(new RefNode(t, name));
}

pNode new_deref_node(pToken t, pNode val)
{
    return pNode(new DerefNode(t, val));
}

pNode new_item_node(pToken t, Symbol v, Vector<pNode> index)
{
    return pNode(new ItemNode(t, v, index));
}

pNode new_if_node(pToken t, pNode cond, pNode body, pNode elsed)
{
    return pNode(new IfNode(t, cond, body, elsed));
}

pNode new_while_node(pToken t, pNode cond, pNode body)
{
    return pNode(new WhileNode(t, cond, body));
}

pNode new_for_node(pToken t, pNode init, pNode cond, pNode exec, pNode body)
{
    return pNode(new ForNode(t, init, cond, exec, body));
}

pNode new_break_node(pToken t)
{
    return pNode(new BreakNode(t));
}

pNode new_return_node(pToken t, pNode ret)
{
    return pNode(new ReturnNode(t, ret));
}

pNode new_return_node(pToken t)
{
    return pNode(new ReturnNode(t));
}

pNode new_continue_node(pToken t)
{
    return pNode(new ContinueNode(t));
}

pNode new_call_node(pToken t, Symbol name, Vector<pNode> ch)
{
    return pNode(new CallNode(t, name, ch));
}

}  // namespace ast
