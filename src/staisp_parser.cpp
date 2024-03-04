#include "staisp_parser.h"

namespace Staisp
{

Token Parser::get_token()
{
    if(_current == _end) {
        my_assert(false, "Error: reaches the end of code.");
    }
    // printf("READ TOKEN ");
    // _current->print(); printf("\n");
    return _current_token = *(_current++);
}

Token Parser::peek() const
{
    return *(_current);
}

Token Parser::current_token() const
{
    if(_current == _begin) {
        my_assert(false, "Error: no current token.");
    }
    return _current_token;
}

void Parser::consume_token(TokenType t)
{
    if(get_token().t != t) {
        error_at_token(current_token(), "[Parser] error 1: token out of expectation");
    }
}

bool Parser::has_token() const
{
    return _current != _end;
}

Ast::AstProg Parser::parser(pCode code)
{
    TokenList list = Lexer().lexer(code);
    return parser(list);
}

bool Parser::is_buildin_sym(Symbol sym)
{
    return gBuildinBinaryOprType.count(sym) || gBuildinSymType.count(sym);
}

Symbol Parser::parse_sym(pEnv env)
{
    get_token();
    if(current_token().t != TOKEN_SYM) {
        error_at_token(current_token(), "[Parser] error 2: not a symbol");
    }
    return current_token().sym;
}

ImmType Parser::parse_type(pEnv env) {
    get_token();
    if(current_token().t != TOKEN_SYM) {
        error_at_token(current_token(), "[Parser] error 3: not a type");
    }
    if(gSymToImmType.count(current_token().sym) == 0) {
        error_at_token(current_token(), "[Parser] error 3: not a type");
    }
    return gSymToImmType.find(current_token().sym)->second;
}

TypedSym Parser::parse_typed_sym(pEnv env)
{
    TypedSym ts;
    ts.tr = parse_type(env);
    consume_token(TOKEN_2DOT);
    ts.sym = parse_sym(env);
    return ts;
}

Ast::pNode Parser::parse_value(pEnv env)
{
    get_token();
    if(current_token().t == TOKEN_INT) {
        return Ast::new_imm_node(current_token(), current_token().val);
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym, env);
    }
    if(!env->count(current_token().sym)) {
        if(peek().t == TOKEN_LB_S) {
            error_at_token(current_token(), "[Parser] error 4: function not found");
        }
        error_at_token(current_token(), "[Parser] error 5: variable not found");
    }
    if((*env)[current_token().sym] == SYM_FUNC) {
        return parse_function_call(current_token().sym, env);
    }
    if(peek().t == TOKEN_LB_S) {
        error_at_token(current_token(), "[Parser] error 6: calling a variable");
    }
    return Ast::new_sym_node(current_token(), current_token().sym);
}

Ast::pNode Parser::parse_buildin_sym(Symbol sym, pEnv env, bool in_global)
{
    auto token = current_token();
    if(gBuildinBinaryOprType.count(sym)) {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::new_opr_node(token, gBuildinBinaryOprType.find(sym)->second, { a1, a2 } );
    }
    switch(gBuildinSymType.find(sym)->second) {
    case BUILDIN_IF: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::new_opr_node(token, Ast::OPR_IF, { a1, a2 } );
    }
    case BUILDIN_IFE: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        auto a3 = parse_value(env);
        return Ast::new_opr_node(token, Ast::OPR_IF, { a1, a2, a3 } );
    }
    case BUILDIN_ASSIGN: {
        auto a1 = parse_sym(env);
        auto a2 = parse_value(env);
        return Ast::new_assign_node(token, a1, a2);
    }
    case BUILDIN_RETURN: {
        auto a1 = parse_value(env);
        return Ast::new_opr_node(token, Ast::OPR_RET, { a1 });
    }
    case BUILDIN_WHILE: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::new_opr_node(token, Ast::OPR_WHILE, { a1, a2 } );
    }
    case BUILDIN_DEFVAR: {
        auto a1 = parse_typed_sym(env);
        auto a2 = parse_value(env);
        if(env->count_current(a1.sym)) {
            error_at_token(current_token(), "[Parser] error 7: definition existed");
        }
        env->set(a1.sym, SYM_VAL);
        return Ast::new_var_def_node(token, a1, a2);
    }
    case BUILDIN_DEFFUNC: {
        if(!in_global) {
            error_at_token(current_token(), "[Parser] error 8: function nested");
        }
        auto a1 = parse_typed_sym(env);
        auto a2 = parse_typed_sym_list(env);
        if(env->count_current(a1.sym)) {
            // should be rejected at error 8
            error_at_token(current_token(), "[Parser] error 7: definition existed - impossible");
        }
        env->set(a1.sym, SYM_FUNC);
        pEnv newEnv = pEnv(new Env(env));
        for(auto i : a2) {
            newEnv->set(i.sym, SYM_VAL);
        }
        auto a3 = parse_statement(newEnv);
        return Ast::new_func_def_node(token, a1, a2, a3);
    }
    case BUILDIN_BLOCK: {
        return parse_block(env);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return nullptr;
}

Ast::pNode Parser::parse_statement(pEnv env)
{
    get_token();
    if(current_token().t != TOKEN_SYM) {
        error_at_token(current_token(), "[Parser] error 9: beginning of a statement must be a symbol");
        return { };
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym, env, true);
    }
    return parse_function_call(current_token().sym, env);
}

Vector<TypedSym> Parser::parse_typed_sym_list(pEnv env)
{
    consume_token(TOKEN_LB_S);
    Vector<TypedSym> ts;
    while(peek().t != TOKEN_RB_S) {
        ts.push_back(parse_typed_sym(env));
    }
    consume_token(TOKEN_RB_S);
    return ts;
}

Ast::AstProg Parser::parse_value_list(pEnv env)
{
    consume_token(TOKEN_LB_S);
    Ast::AstProg list;
    while(peek().t != TOKEN_RB_S) {
        list.push_back(parse_value(env));
    }
    consume_token(TOKEN_RB_S);
    return list;
}

Ast::pNode Parser::parse_function_call(Symbol sym, pEnv env)
{
    if(!env->count(sym)) {
        error_at_token(current_token(), "[Parser] error 4: function not found");
    }
    auto token = current_token();
    switch((*env)[sym]) {
    case SYM_VAL:
        my_assert(false, "Error: cannot reach this point");
        // error_at_token(current_token(), "[Parser] calling a value");
        break;
    case SYM_FUNC: {
        Ast::AstProg args = parse_value_list(env);
        args.push_front(Ast::new_sym_node(token, sym));
        return Ast::new_opr_node(token, Ast::OPR_CALL, args);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return Ast::new_imm_node(current_token(), -1);
}

Ast::pNode Parser::parse_block(pEnv env)
{
    consume_token(TOKEN_LB_L);
    Ast::AstProg body;
    pEnv newEnv = pEnv(new Env(env));
    auto token = current_token();
    while(peek().t != TOKEN_RB_L) {
        body.push_back(parse_statement(newEnv));
    }
    consume_token(TOKEN_RB_L);
    return Ast::new_block_node(token, body);
}

Ast::AstProg Parser::parser(TokenList list, pEnv env)
{
    using namespace Ast;

    AstProg prog;

    _begin = list.begin();
    _current = list.begin();
    _end = list.end();
    while(has_token()) {
        prog.push_back(parse_statement(env));
    }
    return prog;
}

} // namespace staisp
