#pragma once

#include "def.h"

#include "ir_control_instr.h"
#include "ir_instr.h"

namespace Ir {

struct Block;
typedef Pointer<Block> pBlock;
typedef Vector<pBlock> Blocks;

struct Block : public Val {
    Block() : Val(make_void_type()) {}

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
    // 该块的最后一条指令，一般为 ret、br 或者 cond br
    pInstr back() const;

    // 将该块使用到的临时变量存放起来
    // 避免被释放
    // 例如临时数字，因为 Use 本身不是以 shared_ptr 指向 value
    void add_imm(pVal imm);

    // 通过 Label 读取块
    Vector<Block*> in_blocks() const;
    Vector<Block*> out_blocks() const;

    void push_back(pInstr instr);
    // 将 this 与 next 连接起来
    // 只修改 in_block 与 out_block
    void connect(Block *next);

    Instrs body;

    virtual ValType type() const { return VAL_BLOCK; }

    Vector<pVal> imms;
};

struct BlockedProgram {
    // 从 instrs 构建 CFG
    void from_instrs(Instrs &instrs);
    // 在最后一个块上加入最后一条语句
    void push_back(pInstr instr);
    // 重新生成行号信息
    void re_generate() const;

    // 所有的常规优化
    // 包括连接可连接的块
    // 消除死代码
    // 去除无用 basic block
    // 与连接只有强制跳转的 basic block
    void normal_opt();

    // 单个常规优化
    void opt_join_blocks();         // 连接可连接的块
    void opt_remove_dead_code();    // 消除死代码
    void opt_remove_empty_block();  // 去除无用 basic block
    void opt_connect_empty_block(); // 连接只有强制跳转的 basic block

    void opt_trivial();

    // 所有的 basic block
    Blocks blocks;
};

pBlock make_block();

} // namespace Ir
