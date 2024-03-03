#pragma once

#include "def.h"

#include <cctype>

namespace Staisp {

enum TokenType
{
    TOKEN_SYM,
    TOKEN_INT,
    TOKEN_LB_S, // (
    TOKEN_RB_S, // )
    TOKEN_LB_M, // [
    TOKEN_RB_M, // ]
    TOKEN_LB_L, // {
    TOKEN_RB_L, // }
    TOKEN_2DOT, // :
};

struct Token {
    TokenType t;
    union {
        Immediate val;
        Symbol sym;
    };
};

typedef Vector<Token> TokenList;

struct Lexer {
    String::value_type get_char();
    String::value_type peek();
    bool has_char() const;
    void jump_empty();

    TokenList lexer(String code);
    Token lexer_one_token();
    Token lexer_number(String::value_type head);
    Token lexer_sym(String::value_type head);

    String::iterator current;
    String::iterator end;
};

} // namespace staisp
