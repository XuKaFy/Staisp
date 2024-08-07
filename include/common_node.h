// 所有 Ast 的节点的基类
// 每个节点存放的内容包括：
//
// * 节点的类型
// * 节点所属的
// Token（每一个节点都对应一个操作，一个操作对应一个谓词，一个谓词对应一个
// Token）

#pragma once

#include <utility>

#include "common_token.h"
#include "def.h"

enum NodeType {
    NODE_IMM,
    NODE_BINARY,
    NODE_UNARY,
    NODE_CALL,
    NODE_SYM,
    NODE_ASSIGN,
    NODE_DEF_VAR,
    NODE_BASIC_TYPE,
    NODE_POINTER_TYPE,
    NODE_ARRAY_TYPE,
    NODE_DEF_FUNC,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_CAST,
    NODE_REF,
    NODE_DEREF,
    NODE_ITEM,
    NODE_ARRAY_VAL,
};

enum BinaryType {
    OPR_ADD,
    OPR_SUB,
    OPR_MUL,
    OPR_DIV,
    OPR_REM,
    OPR_AND,
    OPR_OR,
    OPR_EQ,
    OPR_NE,
    OPR_GT,
    OPR_GE,
    OPR_LT,
    OPR_LE,
};

enum UnaryType {
    OPR_POS, // nop
    OPR_NEG, // 0 - x     fneg
    OPR_NOT, // xor -1
};

struct Node {
    Node(pToken t, NodeType type);

    void throw_error(int id, const String &object, const String &message);
    virtual void print(size_t tabs = 0) { ; }

    pToken token;
    NodeType type;

    virtual ~Node() = default; // By Yaossg
};

using pNode = Pointer<Node>;
using AstProg =
    List<pNode>; // 可以把一个由 Ast
                 // 组成的程序看作一组节点，每个节点都是全局中的某个操作

struct TypedNodeSym {
    TypedNodeSym(String name, pNode n)
        : name(std::move(std::move(name))), n(std::move(std::move(n))) {}
    void print() {
        n->print();
        printf(" %s", name.c_str());
    }
    String name;
    pNode n;
};
