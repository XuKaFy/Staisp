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

TokenList lexer(String code);

} // namespace staisp
