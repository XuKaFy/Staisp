#include "ast_node.h"

namespace Ast {

pNode new_imm_node(pToken t, Immediate imm, ImmType tr)
{
    return pNode(new ImmNode(t, imm, tr));
}

pNode new_sym_node(pToken t, Symbol str)
{
    return pNode(new SymNode(t, str));
}

pNode new_opr_node(pToken t, OprType type, AstProg ch)
{
    return pNode(new OprNode(t, type, ch));
}

pNode new_assign_node(pToken t, Symbol sym, pNode val)
{
    return pNode(new AssignNode(t, sym, val));
}

pNode new_var_def_node(pToken t, TypedSym var, pNode val)
{
    return pNode(new VarDefNode(t, var, val));
}

pNode new_func_def_node(pToken t, TypedSym var, Vector<TypedSym> args, pNode body, bool is_const)
{
    return pNode(new FuncDefNode(t, var, args, body, is_const));
}

pNode new_block_node(pToken t, AstProg body)
{
    return pNode(new BlockNode(t, body));
}

pNode new_array_def_node(pToken t, Immediates nums)
{
    return pNode(new ArrayDefNode(t, nums));
}

}  // namespace ast
