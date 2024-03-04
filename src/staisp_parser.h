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

typedef Map<String, SymType> Env;

struct Parser {
    Token get_token();
    Token peek();
    void consume_token(TokenType t);
    bool has_token() const;

    static bool is_buildin_sym(Symbol sym);

    Ast::pNode parse_buildin_sym(Symbol sym, Env &env, bool in_global = false);
    Ast::pNode parse_function_call(Symbol sym, Env &env);
    Ast::pNode parse_block(Env &env);
    Ast::pNode parse_statement(Env &env);
    Ast::pNode parse_value(Env &env);
    TypedSym parse_typed_sym(Env &env);
    Symbol parse_sym(Env &env);
    ImmType parse_type(Env &env);
    
    Vector<TypedSym> parse_typed_sym_list(Env &env);
    Ast::AstProg parse_value_list(Env &env);

    Ast::AstProg parser(pCode code);
    Ast::AstProg parser(TokenList list, Env env = {});

    TokenList::iterator current;
    TokenList::iterator end;
};


} // namespace staisp
