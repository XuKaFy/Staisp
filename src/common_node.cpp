#include "common_node.h"

Node::Node(pToken t, NodeType type)
    : token(t), type(type)
{
}

void node_assert(bool judge, pNode root, Symbol message)
{
    if(!judge) {
        root->token->print_error(message);
    }
}

pType join_type(pNode r, pType a1, pType a2)
{
    if(a1->type_type() != a2->type_type()) {
        r->token->print_error("[Type] error 1: type has no joined type");
    }
    switch(a1->type_type()) {
    case TYPE_BASIC_TYPE:
        return make_basic_type(join_imm_type(std::static_pointer_cast<BasicType>(a1)->ty,
                std::static_pointer_cast<BasicType>(a2)->ty), false);
    default: break;
    }
    r->token->print_error("[Type] error 1: type has no joined type");
    return pType();
}
