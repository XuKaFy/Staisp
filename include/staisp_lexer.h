// Staisp 的前端 Lexer 部分
// Lexer 将存放于 String 的代码转换为 TokenList
// 其中，Token 可能有以下类型：
//
// * TOKEN_INT：泛指所有的数字即可能出现在 Ast 中的基本数据类型，例如
// INT64、UINT32、FLOAT64 等
// * TOKEN_SYM：所有可以符合命名规范的单个单词，例如关键字（如
// "DEFVAR"）、变量名（如 "a"）
// * 其余的 TOKEN 类型，到目前为止均为单字符形式
//
// Staisp 使用的 Staisp::StaispToken 继承自 Token，加入了
//
// * TokenType t：指代这个 Token 内存放的额外数据是字符串类型的还是 ImmValue
// 类型的
// * ImmValue val：以 ImmValue 为存放媒介的字面数字常量
// * String sym：以 String 为存放媒介的字面字符串常量

#pragma once

#include "common_token.h"
#include "def.h"
#include "imm.h"

#include <cctype>

namespace Staisp {

enum TokenType {
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
    TOKEN_FLWR, // *
};

struct StaispToken : public Token {
    StaispToken(ImmValue val, pCode code, String::iterator token_begin,
                String::iterator token_end, int line);
    StaispToken(String sym, pCode code, String::iterator token_begin,
                String::iterator token_end, int line);
    StaispToken(TokenType t, String::value_type c, pCode code,
                String::iterator token_begin, String::iterator token_end,
                int line);

    TokenType t;
    ImmValue val;
    String sym;
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
    pToken lexer_float(long long head);

    // _begin 指代当前的 Token 单元从第几个字符开始识别
    String::iterator _begin;
    // _current 指代当前的 Token 单元识别到的最长单元
    // 当对一个 Token 的读取结束的时候，[_begin, _current] 就是一个 Token
    // 的所有信息
    String::iterator _current;
    // _end 是字节流的结束点，用于判断当前的字节流是否结束
    String::iterator _end;
    // 当前代码的共享副本，将会被复制到各个 Token 中
    pCode _p_code;
    // 当前字节流已经到达的代码行数，将会被写入 Token 中
    int _line_count;
};

} // namespace Staisp
