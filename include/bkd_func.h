#pragma once

#include <ir_func.h>
#include <ir_func_defined.h>
#include <set>
#include <type.h>
#include <utility>

#include "bkd_block.h"
#include "def.h"

namespace Backend {

struct StackObject {
    size_t offset, size;
    // and more...
};

/*
layout of stack frame

ra
local variables
spilled registers
caller saved registers
callee saved registers
spilled arguments

*/
struct StackFrame {
    StackObject ra{8, 8};
    size_t offset = 8;
    std::vector<StackObject> locals; // all except ra and args
    size_t push(size_t size) {
        size_t index = locals.size();
        offset += size;
        locals.push_back({offset, size});
        return index;
    }

    std::vector<StackObject> args;
    void at_least_args(size_t n) {
        if (n <= 8) return;
        n -= 8; // first eight arguments are in registers
        for (size_t i = args.size(); i < n; ++i) {
            // rest of arguments then are in stack
            args.push_back({8 + i * 8, 8});
        }
    }
    // don't call this too early
    // and don't forget to call this
    void adjust_args() {
        for (auto& arg : args) {
            arg.offset += offset;
        }
    }

    size_t size(size_t sz = 8) {
        size_t alignment;
        for (auto&& local : locals) sz += local.size;
        alignment = 8;
        if (sz % alignment != 0) sz += alignment - sz % alignment;
        alignment = 16;
        for (auto&& arg : args) sz += arg.size;
        if (sz % alignment != 0) sz += alignment - sz % alignment;
        return sz;
    }
};

struct Func {
    Ir::pFuncDefined ir_func;
    std::string name;
    pFunctionType type;

    std::vector<Block> blocks;
    StackFrame frame;

    explicit Func(Ir::pFuncDefined ir_func)
        : ir_func(std::move(ir_func)) {
        name = this->ir_func->name();
        type = this->ir_func->function_type();
    }

// passes:
    void translate();
    Block translate(const Ir::pBlock &block);
    MachineInstrs translate(const Ir::pInstr &instr);
    Block prolog_prototype();
    void build_block_graph();
    void build_block_def_use();
    void number_instruction(Block* block);
    void liveness_analysis();
    void allocate_hint();
    void allocate_init();
    void allocate_weight();
    void allocate_register();
    void rewrite_operands();
    bool peephole();
    void generate_prolog();
    void remove_pseudo();
    void generate_epilog();
    String generate_asm() const;

    void passes() {
        translate();
        build_block_graph();
        build_block_def_use();
        visited.clear();
        number_instruction(&blocks.front());
        liveness_analysis();
        allocate_hint();
        allocate_init();
        allocate_weight();
        allocate_register();
        rewrite_operands();
        while (peephole()) {}
        generate_prolog();
        remove_pseudo();
        generate_epilog();
    }

    // for translate
    std::unordered_map<int, int> llvmRegToAsmReg;
    int next_reg_{0};
    int convert_reg(int x) {
        if (auto it = llvmRegToAsmReg.find(x); it != llvmRegToAsmReg.end()) {
            return it->second;
        }
        return llvmRegToAsmReg[x] = next_reg();
    }
    int next_reg() {
        return next_reg_++;
    }
    // for numbering
    Set<Block*> visited;
    std::vector<MachineInstr*> num2instr;
    int next_instr_num(MachineInstr* instr) {
        auto index = num2instr.size();
        num2instr.push_back(instr);
        return index;
    }

    struct LiveRange {
        int fromNum, toNum;
        int instr_cnt = 0;
        Block* block;
        bool conflict(const LiveRange &r) const {
            return (fromNum >= r.fromNum && fromNum < r.toNum) || (toNum > r.fromNum && toNum <= r.toNum) ||
                   (fromNum <= r.fromNum && toNum >= r.toNum);
        }
        bool operator<(const LiveRange &r) const {
            if (fromNum != r.fromNum) return fromNum < r.fromNum;
            return toNum < r.toNum;
        }
    };
    Map<GReg, std::vector<LiveRange>> live_ranges;

    enum class AllocStatus {
        New,
        Assign,
        Split,
        Spill,
        Memory,
        Done,
    };

    using AllocPriority = std::tuple<AllocStatus, int, int, bool>;
    using PrioritizedAlloc = std::pair<AllocPriority, int>;
    using AllocRange = std::pair<LiveRange, int>;

    int next_alloc_num_{0};
    int next_alloc_num() {
        return next_alloc_num_++;
    }

    Map<Block*, double> block_weight_map;
    double get_range_weight(const LiveRange& range);
    double get_spill_weight(int alloc_num);

    Map<int, GReg> alloc_map;
    Map<int, GReg> alloc_operand_map;
    Map<int, std::vector<LiveRange>> alloc_range_map;
    Map<int, AllocStatus> alloc_status_map;
    std::priority_queue<PrioritizedAlloc> alloc_priority_queue;

    Map<GReg, Set<int>> occupied_map;
    Map<GReg, std::set<AllocRange>> occupied_range_map;

    // offset in memory.
    Map<GReg, int> operand_spill_map;

    // used temporary registers.
    Set<MachineInstr*> used_temp_map;

    Map<GReg, GReg> coalesce_map;
    Map<GReg, GReg> hint_map;

    void try_allocate(int alloc_num);
    void try_split(int alloc_num);
    void spill(int alloc_num);
    AllocPriority get_alloc_priority(int alloc_num);

    std::set<GReg> saved_registers;
    void add_saved_register(GReg reg);


};

} // namespace Backend
