#pragma once

#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "ir_phi_instr.h"
#include "ir_val.h"
#include <cstddef>
#include <cstdio>
#include <deque>
#include <functional>
#include <set>
#include <utility>

namespace std {
template <typename T1, typename T2> struct hash<std::pair<T1, T2>> {
    size_t operator()(const std::pair<T1, T2> &p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};
} // namespace std

namespace OptGVN {

struct Exp {
    void fold(Map<Exp *, Ir::Const *> &exp_const);
    bool is_folded = false;
    Ir::Instr *instr;
    Vector<Ir::Val *> args; // 变量有顺序，例如 sub

    // 当某个表达式被修改的时候，用到其的所有表达式需要被删除
    // 把所有引用到此表达式的表达式放入 fa 数组
    // 便于更新
    Vector<Ir::Instr *> fa;
};

struct BlockValue {
    bool operator==(const BlockValue &b);
    bool operator!=(const BlockValue &b);

    void cup(const BlockValue &v);

    void clear();

    Set<Exp> exps; // 所有的子表达式
    Map<Ir::Instr *, Exp>
        exp_func; // 某个 Instr 对应的子表达式，用于查找 father
};

struct TransferFunction {
    void operator()(Ir::Block *p, BlockValue &v);
    void operator()(Ir::Block *p, const BlockValue &IN, const BlockValue &OUT);
};
using DomPredicate = Map<Ir::Block *, Set<Ir::Block *>>;

struct exp_eq {
    bool operator()(const Exp &a, const Exp &b) const;
};

class GVN_pass {

    using Edge = Pair<Ir::Block *, Ir::Block *>;

    Alys::DomTree &dom_ctx;
    Ir::BlockedProgram &cur_func;
    Map<Ir::Block *, size_t> rpo_number;

    std::deque<Ir::Block *> rpo{};

    Set<Ir::Val *> touched{};
    Set<Ir::Block *> reachable{};
    DomPredicate dom_set;
    Map<Exp *, Ir::Const *> exp_const;
    Vector<Ir::Instr *> shared_gep;
    Map<Ir::Block *, Vector<Ir::PhiInstr *>> phi_in_blks;

public:
    bool hoist_fold(Ir::Instr *&real_instr, Ir::Instr *&arg_instr);
    void handle_cur_instr(Ir::pInstr cur_instr, Ir::Block *cur_blk,
                          std::function<void(Ir::Instr *)> symbolic_evaluation);
    static Vector<Ir::Val *> collect_usees(Ir::Instr *instr);

    std::set<Exp, exp_eq> exp_pool;
    static bool is_block_definable(Ir::Block *arg_blk, Ir::Instr *arg_instr,
                                   DomPredicate dom_set,
                                   Ir::BlockedProgram &cur_func);
    void ap();
    GVN_pass(Ir::BlockedProgram &arg_func, Alys::DomTree &dom_ctx);
    Exp perform_symbolic_evaluation(Ir::Instr *arg_instr, Ir::Block *arg_blk);
    void perform_congruence_finding(Ir::Instr *arg_instr, Exp *exp);
    void process_out_blks(Ir::Block *arg_blk);
    void sink_gep();
    void phi_predication(Ir::Instr *arg_phi_instr);
};

} // namespace OptGVN
