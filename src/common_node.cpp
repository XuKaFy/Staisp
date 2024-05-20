#include "common_node.h"

Node::Node(pToken t, NodeType type) : token(t), type(type) {}

void Node::throw_error(int id, String object, String message) {
    if (token) {
        token->throw_error(id, object, message);
    } else {
        printf("[%s] Error %d: %s\n", object.c_str(), id, message.c_str());
    }
}
