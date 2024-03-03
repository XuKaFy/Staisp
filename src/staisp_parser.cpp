#include "staisp_parser.h"

namespace Staisp
{

Token Parser::get_token()
{
    if(current == end) {
        my_assert(false, "Error: reaches the end of code.");
    }
    // printf("READ TOKEN ");
    // current->print(); printf("\n");
    return *(current++);
}

Token Parser::peek()
{
    return *(current);
}

void Parser::consume_token(TokenType t)
{
    if(get_token().t != t) {
        // printf("consume %d, but found %d\n", (current-1)->t, t);
        my_assert(false, "Error: token out of expectation.");
    }
}

bool Parser::has_token() const
{
    return current != end;
}

Ast::AstProg Parser::parser(String code)
{
    TokenList list = Lexer().lexer(code);
    return parser(list);
}

bool Parser::is_buildin_sym(Symbol sym)
{
    return gBuildinBinaryOprType.count(sym) || gBuildinSymType.count(sym);
}

Symbol Parser::parse_sym(Env &env)
{
    Token cur = get_token();
    if(cur.t != TOKEN_SYM) {
        my_assert(false, "Error: not a sym");
    }
    return cur.sym;
}

ImmType Parser::parse_type(Env &env) {
    Token cur = get_token();
    if(cur.t != TOKEN_SYM) {
        my_assert(false, "Error: not a type");
    }
    return gSymToImmType.find(cur.sym)->second;
}

TypedSym Parser::parse_typed_sym(Env &env)
{
    TypedSym ts;
    ts.tr = parse_type(env);
    consume_token(TOKEN_2DOT);
    ts.sym = parse_sym(env);
    return ts;
}

Ast::pNode Parser::parse_value(Env &env)
{
    Token cur = get_token();
    if(cur.t == TOKEN_INT) {
        return Ast::newImmNode(cur.val);
    }
    if(is_buildin_sym(cur.sym)) {
        return parse_buildin_sym(cur.sym, env);
    }
    if(!env.count(cur.sym)) {
        printf("Value symbol \"%s\" not found.\n", cur.sym);
        printf("  at token %d\n", end - current);
        my_assert(false, "Error: value cannot be found.");
    }
    switch(env[cur.sym]) {
    case SYM_FUNC: {
        return parse_function_call(cur.sym, env);
    }
    case SYM_VAL: {
        return Ast::newSymNode(cur.sym);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return nullptr;
}

Ast::pNode Parser::parse_buildin_sym(Symbol sym, Env &env, bool in_global)
{
    if(gBuildinBinaryOprType.count(sym)) {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::newOprNode(gBuildinBinaryOprType.find(sym)->second, { a1, a2 } );
    }
    switch(gBuildinSymType.find(sym)->second) {
    case BUILDIN_IF: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::newOprNode(Ast::OPR_IF, { a1, a2 } );
    }
    case BUILDIN_IFE: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        auto a3 = parse_value(env);
        return Ast::newOprNode(Ast::OPR_IF, { a1, a2, a3 } );
    }
    case BUILDIN_ASSIGN: {
        auto a1 = parse_sym(env);
        auto a2 = parse_value(env);
        return Ast::newAssignNode(a1, a2);
    }
    case BUILDIN_RETURN: {
        auto a1 = parse_value(env);
        return Ast::newOprNode(Ast::OPR_RET, { a1 });
    }
    case BUILDIN_WHILE: {
        auto a1 = parse_value(env);
        auto a2 = parse_value(env);
        return Ast::newOprNode(Ast::OPR_WHILE, { a1, a2 } );
    }
    case BUILDIN_DEFVAR: {
        auto a1 = parse_typed_sym(env);
        auto a2 = parse_value(env);
        if(env.count(a1.sym)) {
            my_assert(false, "Error: redefined variant.");
        }
        env[a1.sym] = SYM_VAL;
        return Ast::newVarDefNode(a1, a2);
    }
    case BUILDIN_DEFFUNC: {
        if(!in_global)
            my_assert(false, "Error: function in function.");
        auto a1 = parse_typed_sym(env);
        auto a2 = parse_typed_sym_list(env);
        env[a1.sym] = SYM_FUNC;
        Env newEnv = env;
        for(auto i : a2) {
            newEnv[i.sym] = SYM_VAL;
        }
        auto a3 = parse_statement(newEnv);
        return Ast::newFuncDefNode(a1, a2, a3);
    }
    case BUILDIN_BLOCK: {
        return parse_block(env);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return nullptr;
}

Ast::pNode Parser::parse_statement(Env &env)
{
    Token cur = get_token();
    if(cur.t != TOKEN_SYM) {
        my_assert(false, "Error: synatx error.");
        return { };
    }
    if(is_buildin_sym(cur.sym)) {
        return parse_buildin_sym(cur.sym, env, true);
    }
    return parse_function_call(cur.sym, env);
}

Vector<TypedSym> Parser::parse_typed_sym_list(Env &env)
{
    consume_token(TOKEN_LB_S);
    Vector<TypedSym> ts;
    while(peek().t != TOKEN_RB_S) {
        ts.push_back(parse_typed_sym(env));
    }
    consume_token(TOKEN_RB_S);
    return ts;
}

Ast::AstProg Parser::parse_value_list(Env &env)
{
    consume_token(TOKEN_LB_S);
    Ast::AstProg list;
    while(peek().t != TOKEN_RB_S) {
        list.push_back(parse_value(env));
    }
    consume_token(TOKEN_RB_S);
    return list;
}

Ast::pNode Parser::parse_function_call(Symbol sym, Env &env)
{
    if(!env.count(sym)) {
        my_assert(false, "Error: symbol not found.");
    }
    switch(env[sym]) {
    case SYM_VAL:
        my_assert(false, "Error: try to call a variant.");    
        break;
    case SYM_FUNC: {
        Ast::AstProg args = parse_value_list(env);
        args.push_front(Ast::newSymNode(sym));
        return Ast::newOprNode(Ast::OPR_CALL, args);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return Ast::newImmNode(-1);
}

Ast::pNode Parser::parse_block(Env &env)
{
    consume_token(TOKEN_LB_L);
    Ast::AstProg body;
    Env newEnv = env;
    while(peek().t != TOKEN_RB_L) {
        body.push_back(parse_statement(newEnv));
    }
    consume_token(TOKEN_RB_L);
    return Ast::newBlockNode(body);
}

Ast::AstProg Parser::parser(TokenList list, Env env)
{
    using namespace Ast;

    AstProg prog;

    /*for(auto i : list)
        i.print();
    puts("");*/

    current = list.begin();
    end = list.end();
    while(has_token()) {
        prog.push_back(parse_statement(env));
    }
    return prog;
}

} // namespace staisp
