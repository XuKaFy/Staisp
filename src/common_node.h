#pragma once

#include "def.h"
#include "common_token.h"

enum NodeType
{
    NODE_IMM,
    NODE_OPR,
    NODE_SYM,
    NODE_ASSIGN,
    NODE_DEF_VAR,
    NODE_DEF_FUNC,
    NODE_BLOCK,
};

enum OprType
{
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_SDIV,
    OPR_REM,
    OPR_AND,
    OPR_OR,
    OPR_EQ,
    OPR_NE,
    OPR_UGT,
    OPR_UGE,
    OPR_ULT,
    OPR_ULE,
    OPR_SGT,
    OPR_SGE,
    OPR_SLT,
    OPR_SLE,
    OPR_IF,
    OPR_RET,
    OPR_CALL,
    OPR_WHILE,
    OPR_BREAK,
    OPR_CONTINUE,
};

struct Node
{
    Node(pToken t, NodeType type);

    pToken token;
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef List<pNode> AstProg;
