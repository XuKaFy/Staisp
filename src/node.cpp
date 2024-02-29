#include "node.h"

Pointer<Node> Node::int32(int x)
{
    return Pointer<Node>(new Node {NODE_IMMEDIATE_32, x, {} });
}

Pointer<Node> Node::int64(long long x)
{
    return Pointer<Node>(new Node {NODE_IMMEDIATE_64, x, {} });
}

Pointer<Node> Node::varname(String s)
{
    return Pointer<Node>(new Node {NODE_IMMEDIATE_64, {}, {}, s });
}

Pointer<Node> Node::call(Integer function_id)
{
    return Pointer<Node>(new Node {NODE_CALL, function_id, {} });
}

Pointer<Node> Node::ret()
{
    return Pointer<Node>(new Node {NODE_RET, {}, {} });
}

Pointer<Node> Node::operation_2(OperationType op, Pointer<Node> a[2])
{
    return Pointer<Node>(new Node {NODE_OPERATION_2, op, { a[0], a[1] } });
}

Pointer<Node> Node::operation_2(OperationType op, Pointer<Node> l, Pointer<Node> r)
{
    return Pointer<Node>(new Node {NODE_OPERATION_2, op, { l, r } });
}

