#include "ast_node.h"

#include <utility>

namespace Ast {

pNode new_imm_node(pToken t, ImmValue imm) {
    return pNode(new ImmNode(std::move(t), imm));
}

pNode new_sym_node(pToken t, String str) {
    return pNode(new SymNode(std::move(t), std::move(str)));
}

pNode new_binary_node(pToken t, BinaryType type, pNode lhs, pNode rhs) {
    return pNode(
        new BinaryNode(std::move(t), type, std::move(lhs), std::move(rhs)));
}

pNode new_unary_node(pToken t, UnaryType type, pNode rhs) {
    return pNode(new UnaryNode(std::move(t), type, std::move(rhs)));
}

pNode new_assign_node(pToken t, pNode lv, pNode val) {
    return pNode(new AssignNode(std::move(t), std::move(lv), std::move(val)));
}

pNode new_basic_type_node(pToken t, pType ty) {
    return pNode(new BasicTypeNode(std::move(t), std::move(ty)));
}

pNode new_pointer_type_node(pToken t, pNode n) {
    return pNode(new PointerTypeNode(std::move(t), std::move(n)));
}

pNode new_array_type_node(pToken t, pNode n, pNode index) {
    return pNode(
        new ArrayTypeNode(std::move(t), std::move(n), std::move(index)));
}

pNode new_var_def_node(pToken t, TypedNodeSym var, pNode val, bool is_const) {
    return pNode(
        new VarDefNode(std::move(t), std::move(var), std::move(val), is_const));
}

pNode new_func_def_node(pToken t, TypedNodeSym var, Vector<TypedNodeSym> args,
                        pNode body) {
    return pNode(new FuncDefNode(std::move(t), std::move(var), std::move(args),
                                 std::move(body)));
}

pNode new_block_node(pToken t, AstProg body) {
    return pNode(new BlockNode(std::move(t), std::move(body)));
}

pNode new_array_def_node(pToken t, Vector<pNode> nums) {
    return pNode(new ArrayDefNode(std::move(t), std::move(nums)));
}

pNode new_cast_node(pToken t, pNode ty, pNode val) {
    return pNode(new CastNode(std::move(t), std::move(ty), std::move(val)));
}

pNode new_ref_node(pToken t, pNode v) {
    return pNode(new RefNode(std::move(t), std::move(v)));
}

pNode new_deref_node(pToken t, pNode val) {
    return pNode(new DerefNode(std::move(t), std::move(val)));
}

pNode new_item_node(pToken t, pNode v, Vector<pNode> index) {
    return pNode(new ItemNode(std::move(t), std::move(v), std::move(index)));
}

pNode new_if_node(pToken t, pNode cond, pNode body, pNode elsed) {
    return pNode(new IfNode(std::move(t), std::move(cond), std::move(body),
                            std::move(elsed)));
}

pNode new_while_node(pToken t, pNode cond, pNode body) {
    return pNode(new WhileNode(std::move(t), std::move(cond), std::move(body)));
}

pNode new_for_node(pToken t, pNode init, pNode cond, pNode exec, pNode body) {
    return pNode(new ForNode(std::move(t), std::move(init), std::move(cond),
                             std::move(exec), std::move(body)));
}

pNode new_break_node(pToken t) { return pNode(new BreakNode(std::move(t))); }

pNode new_return_node(pToken t, pNode ret) {
    return pNode(new ReturnNode(std::move(t), std::move(ret)));
}

pNode new_return_node(pToken t) { return pNode(new ReturnNode(std::move(t))); }

pNode new_continue_node(pToken t) {
    return pNode(new ContinueNode(std::move(t)));
}

pNode new_call_node(pToken t, String name, Vector<pNode> ch) {
    return pNode(new CallNode(std::move(t), std::move(name), std::move(ch)));
}

} // namespace Ast
