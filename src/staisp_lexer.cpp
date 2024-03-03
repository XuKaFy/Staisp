#include "staisp_lexer.h"

namespace Staisp
{

String::value_type Lexer::get_char()
{
    if(!has_char()) {
        my_assert(false, "Error: no more chars.");
    }
    return *(current++);
}

String::value_type Lexer::peek()
{
    return *(current);
}

bool Lexer::has_char() const
{
    return current != end;
}

void Lexer::jump_empty()
{
    while(has_char() && isspace(*current)) get_char(); 
}

TokenList Lexer::lexer(String code)
{
    TokenList list;
    current = code.begin();
    end = code.end();
    jump_empty();
    while(has_char()) {
        list.push_back(lexer_one_token());
        jump_empty();
    }
    return list;
}

Token Lexer::lexer_number(String::value_type head)
{
    Immediate var = head - '0';
    while(has_char() && !isspace(peek())) {
        if(!isdigit(peek())) {
            my_assert(false, "Error: not a number.");
        }
        var = var * 10 + get_char() - '0';
    }
    Token t = { TOKEN_INT };
    t.val = var;
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
    return t;
}

Token Lexer::lexer_one_token()
{
    String::value_type ch = get_char();
    switch(ch) {
    case '(': return Token { TOKEN_LB_S, '(' };
    case ')': return Token { TOKEN_RB_S, ')' };
    case '[': return Token { TOKEN_LB_M, '[' };
    case ']': return Token { TOKEN_LB_M, ']' };
    case '{': return Token { TOKEN_LB_L, '{' };
    case '}': return Token { TOKEN_RB_L, '}' };
    case ':': return Token { TOKEN_2DOT, ':' };
    }
    if(isdigit(ch)) {
        return lexer_number(ch);
    }
    return lexer_sym(ch);
}

} // namespace staisp
