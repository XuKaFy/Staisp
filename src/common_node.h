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
    NODE_DEF_CONST_FUNC,
    NODE_BLOCK,
    NODE_CAST,
};

#define OPR_TABLE_CALCULATABLE \
    ENTRY(ADD, +) \
    ENTRY(SUB, -) \
    ENTRY(MUL, *) \
    ENTRY(DIV, /) \
    ENTRY(REM, %) \
    ENTRY(AND, &&) \
    ENTRY(OR, ||) \
    ENTRY(EQ, ==) \
    ENTRY(NE, !=) \
    ENTRY(GT, >) \
    ENTRY(GE, >=) \
    ENTRY(LT, <) \
    ENTRY(LE, <=)

#define OPR_TABLE_UNCALCULATABLE \
    ENTRY(IF) \
    ENTRY(RET) \
    ENTRY(WHILE) \
    ENTRY(BREAK) \
    ENTRY(CONTINUE)


enum OprType
{
#define ENTRY(x, y) OPR_##x,
    OPR_TABLE_CALCULATABLE
#undef ENTRY
#define ENTRY(x) OPR_##x,
    OPR_TABLE_UNCALCULATABLE
#undef ENTRY
    OPR_CALL,
};

struct Node
{
    Node(pToken t, NodeType type);

    pToken token;
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef List<pNode> AstProg;

void node_assert(bool judge, pNode root, Symbol message);
