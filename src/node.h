#pragma once

#include "def.h"

enum NodeType
{
    NODE_OPERATION_2,
    NODE_IMMEDIATE_64,
    NODE_IMMEDIATE_32,
    NODE_VARNAME,
    NODE_ASSIGN,
    NODE_CALL,
    NODE_RET,
};

enum OperationType
{
    NODE_ADD,
    NODE_MUL,
};

struct Node
{
    static Pointer<Node> int32(int x);
    static Pointer<Node> int64(long long x);
    static Pointer<Node> varname(String s);
    static Pointer<Node> operation_2(OperationType op, Pointer<Node> a[2]);
    static Pointer<Node> operation_2(OperationType op, Pointer<Node> l, Pointer<Node> r);
    static Pointer<Node> call(Integer function_id);
    static Pointer<Node> ret();

    NodeType type;
    Integer val;
    Pointer<Node> c[2];
    String name;
};

#define is_immediate(node) (node->type == NODE_IMMEDIATE_32 || node->type == NODE_IMMEDIATE_64)
#define is_assign(node) (node->type == NODE_ASSIGN)
#define is_call(node) (node->type == NODE_CALL)
#define is_ret(node) (node->type == NODE_RET)
#define left_of(node) (node->c[0])
#define right_of(node) (node->c[1])
