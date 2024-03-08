#pragma once

#include "def.h"
#include "env.h"
#include "imm.h"

#include "staisp_lexer.h"
#include "ast_node.h"
#include "ast_exec.h"

namespace Staisp {

#define BUILDIN_SYM_LIST \
    ENTRY(IF) \
    ENTRY(IFE) \
    ENTRY(ASSIGN) \
    ENTRY(RETURN) \
    ENTRY(WHILE) \
    ENTRY(DEFVAR) \
    ENTRY(DEFFUNC) \
    ENTRY(DEFCONSTFUNC) \
    ENTRY(BREAK) \
    ENTRY(CONTINUE) \
    ENTRY(CONSTEXPR) \
    ENTRY(CONST) \
    ENTRY(CAST) \
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
    ENTRY(DIV, div) \
    ENTRY(REM, rem) \
    ENTRY(AND, and) \
    ENTRY(OR, or) \
    ENTRY(EQ, eq) \
    ENTRY(NE, ne) \
    ENTRY(GT, gt) \
    ENTRY(GE, ge) \
    ENTRY(LT, lt) \
    ENTRY(LE, le)

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
    pNode parse_array_def(); // [2 2]
    // pNode parse_array_val(); // { {1 2} {3 4} }
    pNode parse_statement();
    pNode parse_value();
    ImmTypedSym parse_typed_sym();
    Symbol parse_sym();
    ImmType parse_type();
    
    Vector<ImmTypedSym> parse_typed_sym_list();
    AstProg parse_value_list();

    EnvWrapper<SymType> _env;

    TokenList::iterator _begin;
    TokenList::iterator _current;
    TokenList::iterator _end;
    pToken _current_token;
    AstProg _result;
};


} // namespace staisp
