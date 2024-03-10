// 所有 Ast 的节点的基类
// 每个节点存放的内容包括：
//
// * 节点的类型
// * 节点所属的 Token（每一个节点都对应一个操作，一个操作对应一个谓词，一个谓词对应一个 Token）

#pragma once

#include "def.h"
#include "type.h"
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
    NODE_REF,
    NODE_DEREF,
    NODE_ITEM,
    NODE_ARRAY_VAL,
};

// 谓词分为两种
// 一种是可计算的
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

// 一种是不可计算的
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

    void throw_error(int id, Symbol object, Symbol message);

    pToken token;
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef List<pNode> AstProg; // 可以把一个由 Ast 组成的程序看作一组节点，每个节点都是全局中的某个操作
