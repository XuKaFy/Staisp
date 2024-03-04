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
    printf("%s:%d:%d: ", code->file_name.c_str(), t.line, t.token_begin - line_begin + 1);
    printf("error: %s\n", error_code);
    printf("%05d| %s\n", t.line, String(line_begin, line_end).c_str());
    printf("     | ");
    for(size_t i=0; i<t.token_begin-line_begin; ++i)
        putchar(' ');
    for(size_t i=0; i<t.token_end-t.token_begin; ++i)
        putchar('^');
    puts("");
    exit(1);
}

String::value_type Lexer::get_char()
{
    if(!has_char()) {
        my_assert(false, "Error: no more chars.");
    }
    if(peek() == '\n') ++line_count;
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

TokenList Lexer::lexer(pCode code)
{
    TokenList list;
    current = code->p_code->begin();
    end = code->p_code->end();
    line_count = 1;
    p_code = code;
    jump_empty();
    while(has_char()) {
        begin = current;
        list.push_back(lexer_one_token());
        jump_empty();
    }
    p_code = pCode();
    return list;
}

Token Lexer::lexer_number(String::value_type head)
{
    Immediate var = head - '0';
    Token t = { TOKEN_INT };
    t.val = var;
    t.p_code = p_code;
    t.token_begin = begin;
    t.line = line_count;
    while(has_char() && !isspace(peek())) {
        if(!isdigit(peek())) {
            t.token_end = current + 1;
            error_at_token(t, "not a number");
        }
        var = var * 10 + get_char() - '0';
    }
    t.token_end = current;
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
    t.p_code = p_code;
    t.line = line_count;
    t.token_begin = begin;
    t.token_end = current;
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
