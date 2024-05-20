#include "common_node.h"

Node::Node(pToken t, NodeType type) : token(t), type(type) {}

void Node::throw_error(int id, Symbol object, Symbol message) {
    if (token) {
        token->throw_error(id, object, message);
    } else {
        printf("[%s] Error %d: %s\n", object, id, message);
    }
}
