#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <memory>
#include <cstring>
#include <cassert>
#include <list>
#include <utility>

#include <variant>

#define my_assert(x, y) assert(x)

template<typename T>
using Pointer = std::shared_ptr<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T, typename U>
using Map = std::map<T, U>;

typedef long long Immediate;
typedef const char* Symbol;
typedef std::string String;
typedef Pointer<String> pString;

Symbol to_symbol(String s);

#define IMM_TYPE_TABLE \
    ENTRY(0, I1, i1) \
    ENTRY(1, I8, i8) \
    ENTRY(2, I16, i16) \
    ENTRY(3, I32, i32) \
    ENTRY(4, I64, i64)

enum ImmType {
#define ENTRY(x, y, z) IMM_##y = x,
    IMM_TYPE_TABLE
#undef ENTRY
};

const Map<String, ImmType> gSymToImmType {
#define ENTRY(x, y, z) { #z, IMM_##y },
    IMM_TYPE_TABLE
#undef ENTRY
};

ImmType join_type(ImmType a, ImmType b);

#define ENTRY(x, y, z) #z,
const Symbol gImmName[] = {
    IMM_TYPE_TABLE
};
#undef ENTRY

#undef IMM_TYPE_TABLE

struct TypedSym
{
    Symbol sym;
    ImmType tr;
};

template<typename T, typename U>
using Pair = std::pair<T, U>;

namespace Staisp {

enum TokenType
{
    TOKEN_SYM,
    TOKEN_INT,
    TOKEN_LB_S, // (
    TOKEN_RB_S, // )
    TOKEN_LB_M, // [
    TOKEN_RB_M, // ]
    TOKEN_LB_L, // {
    TOKEN_RB_L, // }
    TOKEN_2DOT, // :
    TOKEN_SHRP, // #
};

struct Code {
    pString p_code;
    String file_name;
};

typedef Pointer<Code> pCode;

struct Token {
    TokenType t;
    union {
        Immediate val;
        Symbol sym;
    };

    void print() {
        if(t == TOKEN_SYM) {
            printf("[%s]", sym);
        } else if(t == TOKEN_INT) {
            printf("[%lld]", val);
        } else {
            printf("%c", (char) val);
        }
    }

    pCode p_code;
    String::iterator token_begin;
    String::iterator token_end;
    int line;
};

} // namespace staisp

namespace Ast {

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
};

struct Node
{
    Node(Staisp::Token t, NodeType type)
        : token(t), type(type) { }

    Staisp::Token token;
    NodeType type;
};

typedef Pointer<Node> pNode;
typedef List<pNode> AstProg;

} // namespace ast;
