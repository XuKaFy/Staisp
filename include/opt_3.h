// 优化三：GVN

#pragma once

#include "ir_block.h"

namespace Opt3 {

struct Exp {
    // 判断两个指令是否 “形式上一致”
    // 比如
    // %1 = load %a, %2 = load %b
    // %3 = add %1, %2 和 %4 = add %1, %2
    // %3 和 %4 都会被当作 % = add "a", "b"
    // 当操作数是数字的时候，也是直接按照表达式进行比较
    // 注意，store、br、ret、call 指令不论如何不能是公共子表达式
    // 因为其不参与运算
    bool operator == (const Exp& exp) const;

    // 需要一个比较函数，才能使用 Set 进行查找
    bool operator < (const Exp& exp) const;

    Ir::Instr* instr;
    Vector<String> args; // 变量有顺序，例如 sub

    // 当某个表达式被修改的时候，用到其的所有表达式需要被删除
    // 把所有引用到此表达式的表达式放入 fa 数组
    // 便于更新
    Vector<Ir::Instr*> fa;
};

struct BlockValue {
    bool operator == (const BlockValue &b);
    bool operator != (const BlockValue &b);

    void cup(const BlockValue &v);

    void clear();

    Set<Exp> exps; // 所有的子表达式
    Map<Ir::Instr*, Exp> exp_func; // 某个 Instr 对应的子表达式，用于查找 father
};

struct Utils {
    // transfer function
    void operator () (Ir::Block* p, BlockValue &v);
    void operator () (Ir::Block* p, const BlockValue &IN, const BlockValue &OUT);
};

} // namespace Opt3
