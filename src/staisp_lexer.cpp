#include "staisp_lexer.h"

namespace Staisp
{

StaispToken::StaispToken(Immediate val, pCode code, String::iterator token_begin, 
                         String::iterator token_end, int line)
    : Token(code, token_begin, token_end, line), t(TOKEN_INT), val(val) { }

StaispToken::StaispToken(Symbol sym, pCode code, String::iterator token_begin, 
                         String::iterator token_end, int line)
    : Token(code, token_begin, token_end, line), t(TOKEN_SYM), sym(sym) { }  

StaispToken::StaispToken(TokenType t, String::value_type c, pCode code, String::iterator token_begin, 
                         String::iterator token_end, int line)
    : Token(code, token_begin, token_end, line), t(t), val(c) { }  


String::value_type Lexer::get_char()
{
    if(!has_char()) {
        my_assert(false, "Error: no more chars.");
    }
    if(peek() == '\n') ++_line_count;
    return *(_current++);
}

String::value_type Lexer::peek()
{
    return *(_current);
}

bool Lexer::has_char() const
{
    return _current != _end;
}

void Lexer::jump_empty()
{
    while(has_char() && isspace(peek())) get_char(); 
}

void Lexer::jump_comment()
{
    if(has_char() && peek() == '#') 
        while(has_char() && peek() != '\n') get_char();
}

TokenList Lexer::lexer(pCode code)
{
    TokenList list;
    _current = code->p_code->begin();
    _end = code->p_code->end();
    _line_count = 1;
    _p_code = code;
    while(true) {
        while(has_char() && (isspace(peek()) || peek() == '#')) {
            jump_empty();
            jump_comment();
        }
        if(!has_char()) break;
        _begin = _current;
        list.push_back(lexer_one_token());
        // list.back().print();
    }
    _p_code = pCode();
    return list;
}

pToken Lexer::lexer_number(String::value_type head)
{
    Immediate var = head - '0';
    while(has_char() && !isspace(peek())) {
        if(isalpha(peek())) {
            StaispToken(var, _p_code, _begin, _current, _line_count)
                .print_error("[Lexer] error 1: not a number");
        }
        if(!isdigit(peek()))
            break;
        var = var * 10 + get_char() - '0';
    }
    return pToken(new StaispToken(var, _p_code, _begin, _current, _line_count));
}

pToken Lexer::lexer_sym(String::value_type head)
{
    String sym = "";
    sym += head;
    while(has_char() && !isspace(peek())) {
        if(!isalnum(peek())) break;
        sym += get_char();
    }
    return pToken(new StaispToken(to_symbol(sym), _p_code, _begin, _current, _line_count));
}

pToken Lexer::lexer_one_token()
{
    String::value_type ch = get_char();
    switch(ch) {
    case '(': return pToken(new StaispToken(TOKEN_LB_S, '(', _p_code, _begin, _current, _line_count));
    case ')': return pToken(new StaispToken(TOKEN_RB_S, ')', _p_code, _begin, _current, _line_count));
    case '[': return pToken(new StaispToken(TOKEN_LB_M, '[', _p_code, _begin, _current, _line_count));
    case ']': return pToken(new StaispToken(TOKEN_LB_M, ']', _p_code, _begin, _current, _line_count));
    case '{': return pToken(new StaispToken(TOKEN_LB_L, '{', _p_code, _begin, _current, _line_count));
    case '}': return pToken(new StaispToken(TOKEN_RB_L, '}', _p_code, _begin, _current, _line_count));
    case ':': return pToken(new StaispToken(TOKEN_2DOT, ':', _p_code, _begin, _current, _line_count));
    }
    if(isdigit(ch)) {
        return lexer_number(ch);
    }
    return lexer_sym(ch);
}

} // namespace staisp
