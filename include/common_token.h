// 所有 Token 的基类
// 主要用于精准定位报错位置
// 每个 Token 都需要携带以下信息：
//
// * pCode code：原代码
// * String::iterator begin/end：这个 Token 在代码中的位置
// * int line：这个 Token 在代码中的第几行
//
// 使用这些信息就可以定位错误位置
//
// 理论上所有报错都可以由 Token 的 throw_error 产生
// 其将会向标准输出流中打印一段报错信息，并且抛出一个 Exception

#pragma once

#include "def.h"

struct Token {
public:
    Token(pCode code, String::iterator token_begin, String::iterator token_end,
          int line);

    // 打印报错信息后，throw 一个 Exception
    void throw_error(int id, Symbol object, Symbol error_code);
    // 仅打印警告信息
    void print_warning(int id, Symbol object, Symbol error_code);

private:
    void print_message(Symbol type, int id, Symbol object, Symbol error_code);

    pCode p_code;
    String::iterator token_begin;
    String::iterator token_end;
    int line;
};

typedef Pointer<Token> pToken;
typedef List<pToken> TokenList;
