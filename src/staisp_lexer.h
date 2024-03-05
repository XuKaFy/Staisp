#pragma once

#include "def.h"
#include "common_token.h"

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
    TOKEN_SHRP, // #
};

struct StaispToken : public Token {
    StaispToken(Immediate val, pCode code, 
                String::iterator token_begin, String::iterator token_end, int line);
    StaispToken(Symbol sym, pCode code, 
                String::iterator token_begin, String::iterator token_end, int line);
    StaispToken(TokenType t, String::value_type c, pCode code, 
                String::iterator token_begin, String::iterator token_end, int line);
    
    TokenType t;
    union {
        Immediate val;
        Symbol sym;
    };
};

class Lexer {
public:
    TokenList lexer(pCode code);

private:
    String::value_type get_char();
    String::value_type peek();
    bool has_char() const;
    void jump_empty();
    void jump_comment();

    pToken lexer_one_token();
    pToken lexer_number(String::value_type head);
    pToken lexer_sym(String::value_type head);

    String::iterator _begin;
    String::iterator _current;
    String::iterator _end;
    pCode _p_code;
    int _line_count;
};

} // namespace staisp
