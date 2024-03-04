#pragma once

#include "def.h"

#include "staisp_lexer.h"
#include "ast_node.h"

namespace Staisp {

#define BUILDIN_SYM_LIST \
    ENTRY(IF) \
    ENTRY(IFE) \
    ENTRY(ASSIGN) \
    ENTRY(RETURN) \
    ENTRY(WHILE) \
    ENTRY(DEFVAR) \
    ENTRY(DEFFUNC) \
    ENTRY(BLOCK)

enum BuildinSym
{
#define ENTRY(x) BUILDIN_##x,
    BUILDIN_SYM_LIST
#undef ENTRY
};

const Map<String, BuildinSym> gBuildinSymType {
#define ENTRY(x) { #x, BUILDIN_##x },
    BUILDIN_SYM_LIST
#undef ENTRY
};

#undef BUILDIN_SYM_LIST

#define BUILDIN_BINARY_OPR_LIST \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(MUL, mul) \
    ENTRY(SDIV, sdiv) \
    ENTRY(REM, rem) \
    ENTRY(AND, and) \
    ENTRY(OR, or) \
    ENTRY(EQ, eq) \
    ENTRY(NE, ne) \
    ENTRY(UGT, ugt) \
    ENTRY(UGE, uge) \
    ENTRY(ULT, ult) \
    ENTRY(ULE, ule) \
    ENTRY(SGT, sgt) \
    ENTRY(SGE, sge) \
    ENTRY(SLT, slt) \
    ENTRY(SLE, sle)

enum BuildinBinaryOprType
{
#define ENTRY(x, y) BUILDIN_##x,
    BUILDIN_BINARY_OPR_LIST
#undef ENTRY
};

const Map<String, Ast::OprType> gBuildinBinaryOprType {
#define ENTRY(x, y) { #x, Ast::OPR_##x },
    BUILDIN_BINARY_OPR_LIST
#undef ENTRY
};

#undef BUILDIN_BINARY_OPR_LIST

enum SymType {
    SYM_VAL,
    SYM_FUNC,
};

typedef Map<String, SymType> SymTable;

struct Env;
typedef Pointer<Env> pEnv;

class Env {
public:
    Env(pEnv parent = pEnv())
        : parent(parent) { }

    bool count(Symbol sym) {
        if(table.count(sym)) return true;
        if(parent) return parent->count(sym);
        return false;
    }

    bool count_current(Symbol sym) {
        return table.count(sym);
    }

    void set(Symbol sym, SymType t) {
        table[sym] = t;
    }

    SymType operator [] (Symbol sym) {
        if(table.count(sym)) return table[sym];
        if(parent) return (*parent)[sym];
        my_assert(false, "Error: get no symbol");
        return SYM_VAL;
    }

private:
    pEnv parent {};
    SymTable table {};
};

class Parser {
public:
    static bool is_buildin_sym(Symbol sym);

    Ast::AstProg parser(pCode code);
    Ast::AstProg parser(TokenList list, pEnv env = pEnv(new Env {} ));

private:
    Token get_token();
    Token current_token() const;
    Token peek() const;
    void consume_token(TokenType t);
    bool has_token() const;
    
    Ast::pNode parse_buildin_sym(Symbol sym, pEnv env, bool in_global = false);
    Ast::pNode parse_function_call(Symbol sym, pEnv env);
    Ast::pNode parse_block(pEnv env);
    Ast::pNode parse_statement(pEnv env);
    Ast::pNode parse_value(pEnv env);
    TypedSym parse_typed_sym(pEnv env);
    Symbol parse_sym(pEnv env);
    ImmType parse_type(pEnv env);
    
    Vector<TypedSym> parse_typed_sym_list(pEnv env);
    Ast::AstProg parse_value_list(pEnv env);

    TokenList::iterator _begin;
    TokenList::iterator _current;
    TokenList::iterator _end;
    Token _current_token;
};


} // namespace staisp
