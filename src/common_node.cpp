#include "common_node.h"
#include "def.h"

#include <utility>

Node::Node(pToken t, NodeType type)
    : token(std::move(std::move(t))), type(type) {}

void Node::throw_error(int id, const String &object, const String &message) {
    if (token) {
        token->throw_error(id, object, message);
    } else {
        printf("[%s] Error %d: %s\n", object.c_str(), id, message.c_str());
        my_assert(false, "?");
    }
}
