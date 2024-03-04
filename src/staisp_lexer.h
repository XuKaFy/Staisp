#pragma once

#include "def.h"

#include <cctype>

namespace Staisp {

void error_at_token(Token t, Symbol error_code);

typedef Vector<Token> TokenList;

class Lexer {
public:
    TokenList lexer(pCode code);

private:
    String::value_type get_char();
    String::value_type peek();
    bool has_char() const;
    void jump_empty();

    Token lexer_one_token();
    Token lexer_number(String::value_type head);
    Token lexer_sym(String::value_type head);

    String::iterator _begin;
    String::iterator _current;
    String::iterator _end;
    pCode _p_code;
    int _line_count;
};

} // namespace staisp
