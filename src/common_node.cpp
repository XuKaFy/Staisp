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
