#include "ast_exec.h"

namespace Ast {

Immediate execute(pNode root)
{
    switch(root->type) {
    case NODE_BLOCK:
    case NODE_DEF_FUNC:
    case NODE_DEF_VAR:
    case NODE_SYM:
    case NODE_ASSIGN:
        throw ConstexprException ();
    default: break;
    }
    if(root->type == NODE_IMM)
        return std::static_pointer_cast<ImmNode>(root)->imm;
    // NODE_OPR
    auto r = std::static_pointer_cast<OprNode>(root);
    switch(r->type) {
#define ENTRY(x, y) \
    case OPR_##x: \
        my_assert(r->ch.size() == 2, "Error: constexpr operator has wrong count of args."); \
        return execute(r->ch.front()) y execute(r->ch.back());
    OPR_TABLE_CALCULATABLE
#undef ENTRY
#define ENTRY(x) \
    case OPR_##x:
    OPR_TABLE_UNCALCULATABLE
#undef ENTRY
        throw ConstexprException();
    }
    throw ConstexprException();
}

} // namespace ast
