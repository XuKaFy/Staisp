#pragma once

#include <algorithm>
#include <ir_constant.h>

#include "def.h"

#include "ir_control_instr.h"
#include "ir_instr.h"
#include <utility>

namespace Ir {

struct Block;
struct BlockedProgram;
using pBlock = Pointer<Block>;
using Blocks = List<pBlock>;

struct Block : public Val {
    Block()
        : Val(make_void_type()) {}

    void erase_from_phi();

    // 当该块的指令仅有一条跳转时，将块的 in_block 与 out_block 直接相连
    void connect_in_and_out();
    // 当该块的最后一条指令是 conditional br 时，指定其 cond 内的值，把 cond br
    // 更改为 br
    void squeeze_out(bool selected);
    // 当 out_block 含有 before 时，将其替换为 out（即同时修改 out_block 与 br
    // 指令的指向对象） 最后一条指令必须是 cond br 或者 br
    void replace_out(Block *before, Block *out);

    // 打印该 block
    String print_block() const;

    // 该块的 label，默认为第一条 instr
    Pointer<LabelInstr> label() const;

    // 将该块使用到的临时变量存放起来
    // 避免被释放
    // 例如临时数字，因为 Use 本身不是以 shared_ptr 指向 value
    // 最终所有的 imm 将会放到 BlockedProgram 中
    pVal add_imm(Value value);

    // 通过 Label 读取块
    Set<Block *> in_blocks() const;
    Set<Block *> out_blocks() const;

    // 将 this 与 next 连接起来
    // 只修改 in_block 与 out_block
    void connect(Block *next);

    ValType type() const override { return VAL_BLOCK; }

    BlockedProgram *program() {
        return program_;
    }

    pInstr front() const {
        return body.front();
    }

    pInstr back() const {
        return body.back();
    }

    Instrs::iterator begin() {
        return body.begin();
    }

    Instrs::iterator end() {
        return body.end();
    }

    Instrs::reverse_iterator rbegin() {
        return body.rbegin();
    }

    Instrs::reverse_iterator rend() {
        return body.rend();
    }

    Instrs::iterator insert(const Instrs::iterator &i, const pInstr &instr) {
        instr->set_block(this);
        return body.insert(i, instr);
    }

    void push_back(const pInstr &instr) {
        instr->set_block(this);
        body.push_back(instr);
    }

    void pop_back() {
        body.pop_back();
    }

    Instrs::iterator erase(const Instrs::iterator &i) {
        return body.erase(i);
    }

    void erase(Instr* instr) {
        for (auto i = body.begin(); i != body.end(); ++i) {
            if ((*i).get() == instr) {
                erase(i);
                break;
            }
        }
    }

    void erase(const Instrs::iterator &i, const Instrs::iterator &j) {
        body.erase(i, j);
    }

    void reverse() {
        std::reverse(body.begin(), body.end());
    }

    // 在 Ret 或者 Br 前面插入指令，即倒数第二条
    void push_behind_end(const pInstr &instr);
    // 在 Label 后面插入指令，即正数第二条
    void push_after_label(const pInstr &instr);

    bool empty() const {
        return body.empty();
    }

    size_t size() const {
        return body.size();
    }

private:
    friend struct BlockedProgram;

    void set_program(BlockedProgram* program) {
        program_ = program;
    }

    Instrs body;

    BlockedProgram* program_ {nullptr};
};

pBlock make_block();

struct BlockedProgram {

    void initialize(String name, Instrs instrs, Vector<pVal> args, ConstPool cpool);

    // 是否存在 call 指令
    bool has_calls() const;
    // 重新生成行号信息
    void re_generate() const;
    // 加入一个立即数
    pVal add_imm(Value value);

    // 影响 bb 的普通优化
    void plain_opt_bb();
    // 不影响 bb 的普通优化
    void plain_opt_no_bb();
    // 所有的 plain 优化
    void plain_opt_all();

    bool opt_join_blocks();              // 连接可连接的块
    bool opt_remove_unreachable_block(); // 去除无用 basic block
    bool opt_remove_only_jump_block();   // 连接只有强制跳转的 basic block
    bool opt_simplify_branch();          // 简化分支

    void opt_remove_dead_code();    // 消除死代码
    void opt_trivial();             // 针对个别指令的优化

    // 检查是否存在空的 Use 链
    bool check_empty_use(String state = {});
    // 检查是否有非法的 Phi
    bool check_invalid_phi(String state = {});

    String name() const {
        return name_;
    }

    Blocks::iterator begin() {
        return blocks.begin();
    }

    Blocks::iterator end() {
        return blocks.end();
    }

    Blocks::const_iterator cbegin() const {
        return blocks.cbegin();
    }

    Blocks::const_iterator cend() const {
        return blocks.cend();
    }

    void push_back(const pBlock &block) {
        block->set_program(this);
        blocks.push_back(block);
    }

    Blocks::iterator erase(const Blocks::iterator &i) {
        (*i)->erase_from_phi();
        return blocks.erase(i);
    }

    pBlock front() const {
        return blocks.front();
    }

    pBlock back() const {
        return blocks.back();
    }

    void clear() {
        blocks.clear();
        cpool.clear();
        params_.clear();
    }

    bool empty() const {
        return blocks.empty();
    }

    size_t size() const {
        return blocks.size();
    }

    const Vector<pVal> &params() const {
        return params_;
    }

    ConstPool cpool;
private:
    // 在最后一个块上加入最后一条语句
    void push_back(const pInstr &instr) {
        back()->push_back(instr);
    }

    Vector<pVal> params_;

    // 所有的 basic block
    Blocks blocks;

    // 函数名
    String name_;
};

} // namespace Ir
