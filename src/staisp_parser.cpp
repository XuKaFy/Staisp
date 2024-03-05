#include "staisp_parser.h"

namespace Staisp
{

pEnv Parser::env() {
    if(_env_stack.empty())
        return pEnv();
    return _env_stack.top();
}

void Parser::push_env() {
    _env_stack.push(pEnv(new Env(env())));
}
    
void Parser::end_env() {
    _env_stack.pop();
}
    
void Parser::clear_env() {
    while(!_env_stack.empty())
        _env_stack.pop();
}


bool Env::count(Symbol sym) {
    if(table.count(sym)) return true;
    if(parent) return parent->count(sym);
    return false;
}

bool Env::count_current(Symbol sym) {
    return table.count(sym);
}

void Env::set(Symbol sym, SymType t) {
    table[sym] = t;
}

SymType Env::operator [] (Symbol sym) {
    if(table.count(sym)) return table[sym];
    if(parent) return (*parent)[sym];
    my_assert(false, "Error: get no symbol");
    return SYM_VAL;
}

StaispToken Parser::get_token()
{
    if(_current == _end) {
        my_assert(false, "Error: reaches the end of code.");
    }
    // printf("READ TOKEN ");
    // _current->print(); printf("\n");
    _current_token = *(_current++);
    return current_token();
}

StaispToken Parser::peek() const
{
    return *std::static_pointer_cast<StaispToken>(*_current);
}

StaispToken Parser::current_token() const
{
    if(_current == _begin) {
        my_assert(false, "Error: no current token.");
    }
    return *std::static_pointer_cast<StaispToken>(_current_token);
}

pToken Parser::current_p_token() const
{
    if(_current == _begin) {
        my_assert(false, "Error: no current token.");
    }
    return _current_token;
}

void Parser::consume_token(TokenType t)
{
    if(get_token().t != t) {
        current_token().print_error("[Parser] error 1: token out of expectation");
    }
}

bool Parser::has_token() const
{
    return _current != _end;
}

AstProg Parser::parser(pCode code)
{
    TokenList list = Lexer().lexer(code);
    return parser(list);
}

bool Parser::is_buildin_sym(Symbol sym)
{
    return gBuildinBinaryOprType.count(sym) || gBuildinSymType.count(sym);
}

Symbol Parser::parse_sym()
{
    get_token();
    if(current_token().t != TOKEN_SYM) {
        current_token().print_error("[Parser] error 2: not a symbol");
    }
    return current_token().sym;
}

ImmType Parser::parse_type() {
    get_token();
    if(current_token().t != TOKEN_SYM) {
        current_token().print_error("[Parser] error 3: not a type");
    }
    if(gSymToImmType.count(current_token().sym) == 0) {
        current_token().print_error("[Parser] error 3: not a type");
    }
    return gSymToImmType.find(current_token().sym)->second;
}

TypedSym Parser::parse_typed_sym()
{
    TypedSym ts;
    ts.tr = parse_type();
    consume_token(TOKEN_2DOT);
    ts.sym = parse_sym();
    return ts;
}

pNode Parser::parse_value()
{
    get_token();
    if(current_token().t == TOKEN_INT) {
        return Ast::new_imm_node(current_p_token(), current_token().val);
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym);
    }
    if(!env()->count(current_token().sym)) {
        if(peek().t == TOKEN_LB_S) {
            current_token().print_error("[Parser] error 4: function not found");
        }
        current_token().print_error("[Parser] error 5: variable not found");
    }
    if((*env())[current_token().sym] == SYM_FUNC) {
        return parse_function_call(current_token().sym);
    }
    if(peek().t == TOKEN_LB_S) {
        current_token().print_error("[Parser] error 6: calling a variable");
    }
    return Ast::new_sym_node(current_p_token(), current_token().sym);
}

pNode Parser::parse_buildin_sym(Symbol sym, bool in_global)
{
    auto token = current_p_token();
    if(gBuildinBinaryOprType.count(sym)) {
        auto a1 = parse_value();
        auto a2 = parse_value();
        return Ast::new_opr_node(token, gBuildinBinaryOprType.find(sym)->second, { a1, a2 } );
    }
    switch(gBuildinSymType.find(sym)->second) {
    case BUILDIN_IF: {
        auto a1 = parse_value();
        auto a2 = parse_value();
        return Ast::new_opr_node(token, OPR_IF, { a1, a2 } );
    }
    case BUILDIN_IFE: {
        auto a1 = parse_value();
        auto a2 = parse_value();
        auto a3 = parse_value();
        return Ast::new_opr_node(token, OPR_IF, { a1, a2, a3 } );
    }
    case BUILDIN_ASSIGN: {
        auto a1 = parse_sym();
        auto a2 = parse_value();
        return Ast::new_assign_node(token, a1, a2);
    }
    case BUILDIN_RETURN: {
        auto a1 = parse_value();
        return Ast::new_opr_node(token, OPR_RET, { a1 });
    }
    case BUILDIN_WHILE: {
        auto a1 = parse_value();
        auto a2 = parse_value();
        return Ast::new_opr_node(token, OPR_WHILE, { a1, a2 } );
    }
    case BUILDIN_DEFVAR: {
        auto a1 = parse_typed_sym();
        auto a2 = parse_value();
        if(env()->count_current(a1.sym)) {
            current_token().print_error("[Parser] error 7: definition existed");
        }
        env()->set(a1.sym, SYM_VAL);
        return Ast::new_var_def_node(token, a1, a2);
    }
    case BUILDIN_BREAK:
        return Ast::new_opr_node(token, OPR_BREAK, { });
    case BUILDIN_CONTINUE:
        return Ast::new_opr_node(token, OPR_CONTINUE, { });
    case BUILDIN_CONSTEXPR:
        return Ast::new_imm_node(token, Ast::execute(parse_value()));
    case BUILDIN_DEFFUNC: {
        if(!in_global) {
            current_token().print_error("[Parser] error 8: function nested");
        }
        auto a1 = parse_typed_sym();
        auto a2 = parse_typed_sym_list();
        if(env()->count_current(a1.sym)) {
            // should be rejected at error 8
            current_token().print_error("[Parser] error 7: definition existed - impossible");
        }
        env()->set(a1.sym, SYM_FUNC);
        push_env();
        for(auto i : a2) {
            env()->set(i.sym, SYM_VAL);
        }
        auto a3 = parse_statement();
        end_env();
        return Ast::new_func_def_node(token, a1, a2, a3);
    }
    case BUILDIN_BLOCK: {
        return parse_block();
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return nullptr;
}

pNode Parser::parse_statement()
{
    if(peek().t == TOKEN_LB_L) { // if is start of BLOCK, then process BLOCK without key word
        return parse_block();
    }
    get_token();
    if(current_token().t != TOKEN_SYM) {
        current_token().print_error("[Parser] error 9: beginning of a statement must be a symbol");
        return { };
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym, true);
    }
    return parse_function_call(current_token().sym);
}

Vector<TypedSym> Parser::parse_typed_sym_list()
{
    consume_token(TOKEN_LB_S);
    Vector<TypedSym> ts;
    while(peek().t != TOKEN_RB_S) {
        ts.push_back(parse_typed_sym());
    }
    consume_token(TOKEN_RB_S);
    return ts;
}

AstProg Parser::parse_value_list()
{
    consume_token(TOKEN_LB_S);
    AstProg list;
    while(peek().t != TOKEN_RB_S) {
        list.push_back(parse_value());
    }
    consume_token(TOKEN_RB_S);
    return list;
}

pNode Parser::parse_function_call(Symbol sym)
{
    if(!env()->count(sym)) {
        current_token().print_error("[Parser] error 4: function not found");
    }
    auto token = current_p_token();
    switch((*env())[sym]) {
    case SYM_VAL:
        my_assert(false, "Error: cannot reach this point");
        // current_token().print_error("[Parser] calling a value");
        break;
    case SYM_FUNC: {
        AstProg args = parse_value_list();
        args.push_front(Ast::new_sym_node(token, sym));
        return Ast::new_opr_node(token, OPR_CALL, args);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return Ast::new_imm_node(current_p_token(), -1);
}

pNode Parser::parse_block()
{
    consume_token(TOKEN_LB_L);
    AstProg body;
    auto token = _current_token;
    push_env();
    while(peek().t != TOKEN_RB_L) {
        body.push_back(parse_statement());
    }
    consume_token(TOKEN_RB_L);
    end_env();
    return Ast::new_block_node(token, body);
}

pNode Parser::parse_array_def()
{
    consume_token(TOKEN_LB_M);
    auto token = _current_token;
    Immediates nums;
    while(peek().t != TOKEN_RB_M) {
        nums.push_back(Ast::execute(parse_value()));
    }
    consume_token(TOKEN_RB_M);
    return Ast::new_array_def_node(token, nums);
}

AstProg Parser::parser(TokenList list)
{
    AstProg prog;

    _begin = list.begin();
    _current = list.begin();
    _end = list.end();
    clear_env();
    push_env();
    while(has_token()) {
        prog.push_back(parse_statement());
    }
    end_env();
    return prog;
}

} // namespace staisp
