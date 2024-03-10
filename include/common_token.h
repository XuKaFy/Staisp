#pragma once

#include "def.h"

struct TokenException { };

struct Token 
{
public:
    Token(pCode code, String::iterator token_begin, String::iterator token_end, int line);

    void print_error(Symbol error_code);

private:
    pCode p_code;
    String::iterator token_begin;
    String::iterator token_end;
    int line;
};

typedef Pointer<Token> pToken;
typedef List<pToken> TokenList;
