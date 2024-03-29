// Staisp 前端的 Parser，将 TokenList 转化为 AstProg
// 将 Token 转化为 Ast 的文法已经写在 README.md
//
// 需要特别说明的是，在计算数组大小时，要求数组大小在编译器可计算出
// 计算任务交由 Executor 实现

#pragma once

#include "def.h"
#include "env.h"
#include "imm.h"
#include "type.h"

#include "staisp_lexer.h"
#include "ast_node.h"
#include "ast_exec.h"

namespace Staisp {

// 关键字列表
#define BUILDIN_SYM_LIST \
    ENTRY(IF) \
    ENTRY(IFE) \
    ENTRY(ASSIGN) \
    ENTRY(RETURN) \
    ENTRY(WHILE) \
    ENTRY(DEFVAR) \
    ENTRY(DEFFUNC) \
    ENTRY(DEFCONSTFUNC) \
    ENTRY(BREAK) \
    ENTRY(CONTINUE) \
    ENTRY(CONSTEXPR) \
    ENTRY(CONST) \
    ENTRY(CAST) \
    ENTRY(REF) \
    ENTRY(DEREF) \
    ENTRY(ITEM) \
    ENTRY(BLOCK)

// 所有关键字组成的枚举
enum BuildinSym
{
#define ENTRY(x) BUILDIN_##x,
    BUILDIN_SYM_LIST
#undef ENTRY
};

// 将关键字映射到节点类型
const Map<String, BuildinSym> gBuildinSymType {
#define ENTRY(x) { #x, BUILDIN_##x },
    BUILDIN_SYM_LIST
#undef ENTRY
};

#undef BUILDIN_SYM_LIST

// 所有二元计算的节点类型
#define BUILDIN_BINARY_OPR_LIST \
    ENTRY(ADD, add) \
    ENTRY(SUB, sub) \
    ENTRY(MUL, mul) \
    ENTRY(DIV, div) \
    ENTRY(REM, rem) \
    ENTRY(AND, and) \
    ENTRY(OR, or) \
    ENTRY(EQ, eq) \
    ENTRY(NE, ne) \
    ENTRY(GT, gt) \
    ENTRY(GE, ge) \
    ENTRY(LT, lt) \
    ENTRY(LE, le)

// 所有二元计算的枚举
enum BuildinBinaryOprType
{
#define ENTRY(x, y) BUILDIN_##x,
    BUILDIN_BINARY_OPR_LIST
#undef ENTRY
};

// 将二元运算关键字转化为**操作节点**类型
const Map<String, OprType> gBuildinBinaryOprType {
#define ENTRY(x, y) { #x, OPR_##x },
    BUILDIN_BINARY_OPR_LIST
#undef ENTRY
};

#undef BUILDIN_BINARY_OPR_LIST

// 一个标识符，只能是值或者函数
enum SymType {
    SYM_VAL,
    SYM_FUNC,
};

class Parser {
public:
    // 判断一个 C 风格字符串内容是否为关键字
    static bool is_buildin_sym(Symbol sym);

    AstProg parse(pCode code);
    AstProg parse(TokenList list);

private:
    // 注意，所有的报错都是从当前 Token 爆出
    // 可以这么理解，若错误是之前的 Token 爆出的，那么就不应该执行到下个 Token
    // 如果是之后的 Token 爆出，那么错误就不在当前节点
    // 因此，被爆出的错误只能是当前 Token 产生
    void throw_error(int id, Symbol msg);

    StaispToken get_token();
    StaispToken current_token() const;
    pToken current_p_token() const;
    StaispToken peek() const;
    Pointer<StaispToken> peek_p() const;
    void consume_token(TokenType t);
    bool has_token() const;
    
    pNode parse_buildin_sym(Symbol sym, bool in_global);
    pNode parse_function_call(Symbol sym);
    pNode parse_block();
    pNode parse_statement(bool in_global);
    pNode parse_value();
    Vector<pNode> parse_array_value();
    pNode parse_left_value();
    TypedSym parse_typed_sym();
    Symbol parse_sym();
    pNode parse_sym_node();
    pType parse_type();
    ImmValue parse_single_value_list();
    
    bool is_const_symbol() const;
    bool is_type_ended() const;

    Vector<TypedSym> parse_typed_sym_list();
    AstProg parse_value_list();

    // 用于验证标识符可达性的栈环境
    // 注意，只检查一个标识符是函数还是值
    // 在 Parser 中不对值类型进行检验
    EnvWrapper<SymType> _env;

    TokenList::iterator _begin;
    TokenList::iterator _current;
    TokenList::iterator _end;
    pToken _current_token;
    AstProg _result;
};


} // namespace staisp
