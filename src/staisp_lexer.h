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

struct Code {
    pString p_code;
    String file_name;
};

typedef Pointer<Code> pCode;

struct Token {
    TokenType t;
    union {
        Immediate val;
        Symbol sym;
    };
    pCode p_code;
    int line;
    String::iterator token_begin;
    String::iterator token_end;

    void print() {
        if(t == TOKEN_SYM) {
            printf("[%s]", sym);
        } else if(t == TOKEN_INT) {
            printf("[%lld]", val);
        } else {
            printf("%c", (char) val);
        }
    }
};

void error_at_token(Token t, Symbol error_code);

typedef Vector<Token> TokenList;

struct Lexer {
    String::value_type get_char();
    String::value_type peek();
    bool has_char() const;
    void jump_empty();

    TokenList lexer(pCode code);
    Token lexer_one_token();
    Token lexer_number(String::value_type head);
    Token lexer_sym(String::value_type head);

    String::iterator begin;
    String::iterator current;
    String::iterator end;
    pCode p_code;
    int line_count;
};

} // namespace staisp
