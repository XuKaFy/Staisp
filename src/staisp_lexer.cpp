#include "staisp_lexer.h"

namespace Staisp
{

void error_at_token(Token t, Symbol error_code)
{
    pCode code = t.p_code;
    String::iterator line_begin = t.token_begin;
    String::iterator line_end = t.token_end;
    while(*line_begin != '\n' && line_begin != code->p_code->begin()) --line_begin;
    while(*line_end != '\n' && line_end != code->p_code->end()) ++line_end;
    if(*line_begin == '\n') ++line_begin;
    printf("%s:%d:%lld: %s\n", code->file_name.c_str(), t.line, t.token_begin - line_begin + 1, error_code);
    printf("%5d | %s\n        ", t.line, String(line_begin, line_end).c_str());
    for(String::difference_type i=0; i<t.token_begin-line_begin; ++i)
        putchar(' ');
    for(String::difference_type i=0; i<t.token_end-t.token_begin; ++i)
        putchar('^');
    puts("");
    exit(1);
}

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
    while(has_char() && isspace(*_current)) get_char(); 
}

TokenList Lexer::lexer(pCode code)
{
    TokenList list;
    _current = code->p_code->begin();
    _end = code->p_code->end();
    _line_count = 1;
    _p_code = code;
    jump_empty();
    while(has_char()) {
        _begin = _current;
        list.push_back(lexer_one_token());
        jump_empty();
    }
    _p_code = pCode();
    return list;
}

Token Lexer::lexer_number(String::value_type head)
{
    Immediate var = head - '0';
    Token t = { TOKEN_INT };
    t.val = var;
    t.p_code = _p_code;
    t.token_begin = _begin;
    t.line = _line_count;
    while(has_char() && !isspace(peek())) {
        if(!isdigit(peek())) {
            t.token_end = _current + 1;
            error_at_token(t, "[Lexer] error 1: not a number");
        }
        var = var * 10 + get_char() - '0';
    }
    t.token_end = _current;
    return t;
}

Token Lexer::lexer_sym(String::value_type head)
{
    String sym = "";
    sym += head;
    while(has_char() && !isspace(peek())) {
        if(!isalnum(peek())) break;
        sym += get_char();
    }
    Token t = { TOKEN_SYM };
    t.sym = to_symbol(sym);
    t.p_code = _p_code;
    t.line = _line_count;
    t.token_begin = _begin;
    t.token_end = _current;
    return t;
}

Token Lexer::lexer_one_token()
{
    String::value_type ch = get_char();
    switch(ch) {
    case '(': return Token { TOKEN_LB_S, '(', _p_code, _begin, _current, _line_count };
    case ')': return Token { TOKEN_RB_S, ')', _p_code, _begin, _current, _line_count };
    case '[': return Token { TOKEN_LB_M, '[', _p_code, _begin, _current, _line_count };
    case ']': return Token { TOKEN_LB_M, ']', _p_code, _begin, _current, _line_count };
    case '{': return Token { TOKEN_LB_L, '{', _p_code, _begin, _current, _line_count };
    case '}': return Token { TOKEN_RB_L, '}', _p_code, _begin, _current, _line_count };
    case ':': return Token { TOKEN_2DOT, ':', _p_code, _begin, _current, _line_count };
    }
    if(isdigit(ch)) {
        return lexer_number(ch);
    }
    return lexer_sym(ch);
}

} // namespace staisp
