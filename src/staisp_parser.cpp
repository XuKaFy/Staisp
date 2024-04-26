#include "staisp_parser.h"

namespace Staisp
{

void Parser::throw_error(int id, Symbol msg)
{
    current_token().throw_error(id, "Parser", msg);
}

StaispToken Parser::get_token()
{
    if(_current == _end) {
        my_assert(false, "Error: reaches the end of code.");
    }
    // printf("READ TOKEN ");
    // _current->name(); printf("\n");
    _current_token = *(_current++);
    return current_token();
}

StaispToken Parser::peek() const
{
    return *std::static_pointer_cast<StaispToken>(*_current);
}

Pointer<StaispToken> Parser::peek_p() const
{
    return std::static_pointer_cast<StaispToken>(*_current);
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
        throw_error(1, "token out of expectation");
    }
}

bool Parser::has_token() const
{
    return _current != _end;
}

AstProg Parser::parse(pCode code)
{
    TokenList list = Lexer().lexer(code);
    return parse(list);
}

bool Parser::is_buildin_sym(Symbol sym)
{
    return gBuildinBinaryOprType.count(sym) || gBuildinSymType.count(sym);
}

pNode Parser::parse_sym_node()
{
    get_token();
    if(current_token().t != TOKEN_SYM) {
        throw_error(2, "not a symbol");
    }
    return Ast::new_sym_node(current_p_token(), current_token().sym);
}

Symbol Parser::parse_sym()
{
    get_token();
    if(current_token().t != TOKEN_SYM) {
        throw_error(2, "not a symbol");
    }
    return current_token().sym;
}

bool Parser::is_type_ended() const
{
    if(peek().t == TOKEN_FLWR || peek().t == TOKEN_LB_M)
        return false;
    return true;
}

pType Parser::parse_type() {
    get_token();
    if(current_token().t != TOKEN_SYM) {
        throw_error(3, "not a type");
    }
    if(strcmp(current_token().sym, "void") == 0) {
        return make_void_type();
    }
    if(gSymToImmType.count(current_token().sym) == 0) {
        throw_error(3, "not a type");
    }
    pType root = make_basic_type(gSymToImmType.find(current_token().sym)->second);
    // 注意，对 Type 的识别是左递归的
    while(!is_type_ended()) {
        if(peek().t == TOKEN_FLWR) {
            root = make_pointer_type(root);
            consume_token(TOKEN_FLWR);
            continue;
        }
        ImmValue val = parse_single_value_list();
        if(!is_imm_integer(val.ty)) {
            throw_error(13, "type of index should be integer");
        }
        if(is_imm_signed(val.ty)) {
            root = make_array_type(root, val.val.ival);
        } else {
            root = make_array_type(root, val.val.uval);
        }
    }
    return root;
}

ImmValue Parser::parse_single_value_list()
{
    consume_token(TOKEN_LB_M);
    // 数组大小必须是可以计算出的
    pNode node = parse_value();
    if(node->type != NODE_IMM) {
        throw_error(13, "type of index should be integer");
    }
    consume_token(TOKEN_RB_M);
    return std::static_pointer_cast<Ast::ImmNode>(node)->imm;
}

TypedSym Parser::parse_typed_sym()
{
    auto type = parse_type();
    consume_token(TOKEN_2DOT);
    auto sym = parse_sym();
    return TypedSym(sym, type);
}

Vector<pNode> Parser::parse_array_value()
{
    consume_token(TOKEN_LB_M);
    Vector<pNode> ts;
    while(peek().t != TOKEN_RB_M) {
        if(peek().t == TOKEN_LB_M) {
            ts.push_back(Ast::new_array_def_node(peek_p(), parse_array_value()));
        } else {
            ts.push_back(parse_value());
        }
    }
    consume_token(TOKEN_RB_M);
    return ts;
}

pNode Parser::parse_value()
{
    if(peek().t == TOKEN_LB_M) {
        auto i = peek_p();
        return Ast::new_array_def_node(i, parse_array_value());
    }
    get_token();
    if(current_token().t == TOKEN_INT) {
        return Ast::new_imm_node(current_p_token(), current_token().val);
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym);
    }
    if(!_env.env()->count(current_token().sym)) {
        if(peek().t == TOKEN_LB_S) {
            throw_error(4, "function not found");
        }
        throw_error(5, "variable not found");
    }
    if((*_env.env())[current_token().sym] == SYM_FUNC) {
        return parse_function_call(current_token().sym);
    }
    if(peek().t == TOKEN_LB_S) {
        throw_error(6, "calling a variable");
    }
    return Ast::new_sym_node(current_p_token(), current_token().sym);
}

pNode Parser::parse_left_value()
{
    auto nxt = peek();
    if(nxt.t == TOKEN_INT) {
        throw_error(12, "expected to be a left-value [1]");
    }
    if(gBuildinSymType.count(nxt.sym)) {
        get_token();
        auto r = gBuildinSymType.find(current_token().sym)->second;
        if(r == BUILDIN_DEREF) {
            return Ast::new_deref_node(current_p_token(), parse_value());
        }
        if(r == BUILDIN_ITEM) {
            auto name = parse_sym();
            auto index = parse_array_value();
            return Ast::new_item_node(current_p_token(), name, index);
        }
        throw_error(12, "expected to be a left-value [2]");
    }
    return parse_sym_node();
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
        auto a2 = parse_statement(false);
        return Ast::new_if_node(token, a1, a2);
    }
    case BUILDIN_IFE: {
        auto a1 = parse_value();
        auto a2 = parse_statement(false);
        auto a3 = parse_statement(false);
        return Ast::new_if_node(token, a1, a2, a3);
    }
    case BUILDIN_ASSIGN: {
        auto a1 = parse_left_value();
        auto a2 = parse_value();
        return Ast::new_assign_node(token, a1, a2);
    }
    case BUILDIN_RETURN: {
        auto a1 = parse_value();
        return Ast::new_return_node(token, a1);
    }
    case BUILDIN_RET_VOID: {
        return Ast::new_return_node(token);
    }
    case BUILDIN_WHILE: {
        auto a1 = parse_value();
        auto a2 = parse_statement(false);
        return Ast::new_while_node(token, a1, a2);
    }
    case BUILDIN_FOR: {
        // FOR (DEFVAR i32:i 0
        //      LT i 10)
        //      ASSIGN i ADD i 1)
        consume_token(TOKEN_LB_S);
        auto a1 = parse_statement(false);
        auto a2 = parse_value();
        auto a3 = parse_statement(false);
        consume_token(TOKEN_RB_S);
        auto a4 = parse_statement(false);
        return Ast::new_for_node(token, a1, a2, a3, a4);
    }
    case BUILDIN_DEFVAR: {
        auto a1 = parse_typed_sym();
        auto a2 = parse_value();
        if(_env.env()->count_current(a1.sym)) {
            throw_error(7, "definition existed");
        }
        _env.env()->set(a1.sym, SYM_VAL);
        return Ast::new_var_def_node(token, a1, a2);
    }
    case BUILDIN_BREAK:
        return Ast::new_break_node(token);
    case BUILDIN_CONTINUE:
        return Ast::new_continue_node(token);
    case BUILDIN_DEFFUNC: {
        if(!in_global) {
            throw_error(8, "function nested");
        }
        auto a1 = parse_typed_sym();
        auto a2 = parse_typed_sym_list();
        if(_env.env()->count_current(a1.sym)) {
            // should be rejected at error 8
            throw_error(-1, "definition existed - impossible");
        }
        _env.env()->set(a1.sym, SYM_FUNC);
        _env.push_env();
        for(auto i : a2) {
            _env.env()->set(i.sym, SYM_VAL);
        }
        auto a3 = parse_statement(false);
        _env.end_env();
        return Ast::new_func_def_node(token, a1, a2, a3);
    }
    case BUILDIN_BLOCK: {
        return parse_block();
    }
    case BUILDIN_CAST: {
        auto type = parse_type();
        return Ast::new_cast_node(token, type, parse_value());
    }
    case BUILDIN_REF: {
        return Ast::new_ref_node(token, parse_sym());
    }
    case BUILDIN_DEREF: {
        return Ast::new_deref_node(token, parse_value());
    }
    case BUILDIN_ITEM: {
        auto v = parse_sym();
        auto index = parse_array_value();
        return Ast::new_item_node(token, v, index);
    }
    }
    // cannot reach
    my_assert(false, "Error: cannot reach.");
    return nullptr;
}

pNode Parser::parse_statement(bool in_global)
{
    if(peek().t == TOKEN_LB_L) { // if is start of BLOCK, then process BLOCK without key word
        return parse_block();
    }
    get_token();
    if(current_token().t != TOKEN_SYM) {
        throw_error(9, "beginning of a statement must be a symbol");
        return { };
    }
    if(is_buildin_sym(current_token().sym)) {
        return parse_buildin_sym(current_token().sym, in_global);
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

Vector<pNode> Parser::parse_value_list()
{
    consume_token(TOKEN_LB_S);
    Vector<pNode> list;
    while(peek().t != TOKEN_RB_S) {
        list.push_back(parse_value());
    }
    consume_token(TOKEN_RB_S);
    return list;
}

pNode Parser::parse_function_call(Symbol sym)
{
    if(!_env.env()->count(sym)) {
        throw_error(4, "function not found");
    }
    auto token = current_p_token();
    switch((*_env.env())[sym]) {
    case SYM_VAL:
        my_assert(false, "Error: cannot reach this point");
        // current_token().throw_error("[Parser] calling a value");
        break;
    case SYM_FUNC: {
        Vector<pNode> args = parse_value_list();
        return Ast::new_call_node(token, sym, args);
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
    _env.push_env();
    while(peek().t != TOKEN_RB_L) {
        body.push_back(parse_statement(false));
    }
    consume_token(TOKEN_RB_L);
    _env.end_env();
    return Ast::new_block_node(token, body);
}

AstProg Parser::parse(TokenList list)
{
    try {
        _result.clear();
        _begin = list.begin();
        _current = list.begin();
        _end = list.end();
        _env.clear_env();
        _env.push_env();
        while(has_token()) {
            _result.push_back(parse_statement(true));
        }
        _env.end_env();
        return _result;
    } catch (Exception e) {
        current_token().throw_error(e.id, e.object, e.message);
    }
    return {};
}

} // namespace staisp
