#include "common_token.h"

Token::Token(pCode code, String::iterator token_begin, String::iterator token_end, int line)
    : p_code(code), token_begin(token_begin), token_end(token_end), line(line)
{
}

void Token::print_error(Symbol error_code)
{
    String::iterator line_begin = token_begin;
    String::iterator line_end = token_end;
    while(*line_begin != '\n' && line_begin != p_code->p_code->begin()) --line_begin;
    while(*line_end != '\n' && line_end != p_code->p_code->end()) ++line_end;
    if(*line_begin == '\n') ++line_begin;

    printf("%s:%d:%lld: %s\n", p_code->file_name.c_str(), line, token_begin - line_begin + 1, error_code);
    printf("%5d | %s\n        ", line, String(line_begin, line_end).c_str());
    for(String::difference_type i=0; i<token_begin-line_begin; ++i)
        putchar(' ');
    for(String::difference_type i=0; i<token_end-token_begin; ++i)
        putchar('^');
    puts("");

    throw TokenException();
}
