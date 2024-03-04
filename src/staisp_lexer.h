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

    void print() {
        if(t == TOKEN_SYM) {
            printf("[%s]", sym);
        } else if(t == TOKEN_INT) {
            printf("[%lld]", val);
        } else {
            printf("%c", (char) val);
        }
    }

    pCode p_code;
    String::iterator token_begin;
    String::iterator token_end;
    int line;
};

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
