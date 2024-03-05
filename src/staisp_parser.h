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
    ENTRY(BREAK) \
    ENTRY(CONTINUE) \
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

const Map<String, OprType> gBuildinBinaryOprType {
#define ENTRY(x, y) { #x, OPR_##x },
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
typedef Stack<pEnv> EnvStack;

class Env {
public:
    Env(pEnv parent = pEnv())
        : parent(parent) { }

    bool count(Symbol sym);
    bool count_current(Symbol sym);
    void set(Symbol sym, SymType t);
    SymType operator [] (Symbol sym);

private:
    pEnv parent {};
    SymTable table {};
};

class Parser {
public:
    static bool is_buildin_sym(Symbol sym);

    AstProg parser(pCode code);
    AstProg parser(TokenList list);

private:
    StaispToken get_token();
    StaispToken current_token() const;
    pToken current_p_token() const;
    StaispToken peek() const;
    void consume_token(TokenType t);
    bool has_token() const;
    
    pNode parse_buildin_sym(Symbol sym, bool in_global = false);
    pNode parse_function_call(Symbol sym);
    pNode parse_block();
    pNode parse_statement();
    pNode parse_value();
    TypedSym parse_typed_sym();
    Symbol parse_sym();
    ImmType parse_type();
    
    Vector<TypedSym> parse_typed_sym_list();
    AstProg parse_value_list();

    pEnv env();
    void push_env();
    void end_env();
    void clear_env();

    EnvStack _env_stack;

    TokenList::iterator _begin;
    TokenList::iterator _current;
    TokenList::iterator _end;
    pToken _current_token;
};


} // namespace staisp
