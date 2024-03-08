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

pNode new_opr_node(pToken t, OprType type, AstProg ch)
{
    return pNode(new OprNode(t, type, ch));
}

pNode new_assign_node(pToken t, Symbol sym, pNode val)
{
    return pNode(new AssignNode(t, sym, val));
}

pNode new_var_def_node(pToken t, ImmTypedSym var, pNode val)
{
    return pNode(new VarDefNode(t, var, val));
}

pNode new_func_def_node(pToken t, ImmTypedSym var, Vector<ImmTypedSym> args, pNode body, bool is_const)
{
    return pNode(new FuncDefNode(t, var, args, body, is_const));
}

pNode new_block_node(pToken t, AstProg body)
{
    return pNode(new BlockNode(t, body));
}

pNode new_array_def_node(pToken t, ImmValues nums)
{
    return pNode(new ArrayDefNode(t, nums));
}

pNode new_cast_node(pToken t, ImmType ty, pNode val)
{
    return pNode(new CastNode(t, ty, val));
}

pNode new_ref_node(pToken t, Symbol name)
{
    return pNode(new RefNode(t, name));
}

pNode new_deref_node(pToken t, ImmType ty, pNode val)
{
    return pNode(new DerefNode(t, ty, val));
}

}  // namespace ast
