#pragma once

#include <ir_func.h>
#include <ir_func_defined.h>
#include <set>
#include <type.h>
#include <utility>

#include "bkd_block.h"
#include "def.h"

extern bool flag_O1;

namespace Backend {

struct StackObject {
    int offset, size;
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
    int offset = 8;
    std::vector<StackObject> locals; // all except ra and args
    size_t push(int size) {
        size_t index = locals.size();
        if (size == 8 && offset % 8 == 4) offset += 4;
        offset += size;
        locals.push_back({offset, size});
        return index;
    }

    size_t args = 0;

    int size() const {
        int sz = offset;
        int alignment;
        alignment = 8;
        if (sz % alignment != 0) sz += alignment - sz % alignment;
        sz += args * 8;
        alignment = 16;
        if (sz % alignment != 0) sz += alignment - sz % alignment;
        return sz;
    }
};

struct Func {
    Ir::pFuncDefined ir_func;
    std::string name;
    pFunctionType type;

    std::deque<Block> blocks;
    StackFrame frame;

    explicit Func(Ir::pFuncDefined ir_func)
        : ir_func(std::move(ir_func)) {
        name = this->ir_func->name();
        type = this->ir_func->function_type();
    }

// passes:
    void translate();
    Block translate(const Ir::pBlock &block);
    MachineInstrs translate(const Ir::pBlock &block, const Ir::pInstr &instr, const Ir::pInstr &next, bool& tail);
    Block generate_prolog_tail();
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
        preprocess();
        translate();
        build_block_graph();
        build_block_def_use();
        visited.clear();
        number_instruction(&blocks.front());
        liveness_analysis();
        if (flag_O1) allocate_hint();
        allocate_init();
        allocate_weight();
        allocate_register();
        rewrite_operands();
        generate_prolog();
        remove_pseudo();
        generate_epilog();
        if (flag_O1) while (peephole()) {}
    }

    // for translate
    Map<Ir::Instr*, size_t> localIndex;
    Map<int, int> llvmRegToAsmReg;
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

    void preprocess() {
        int args_size = type->arg_type.size();
        for (int i = 0; i < args_size; ++i) {
            llvmRegToAsmReg[i] = i;
        }
        next_reg_ = args_size;
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
    struct PrioritizedAlloc {
        int alloc_num;
        AllocPriority priority;

        bool operator<(PrioritizedAlloc const& other) const {
            return priority < other.priority;
        }
    };
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
    Map<GReg, size_t> operand_spill_map;

    // used temporary registers.
    Set<MachineInstr*> used_temp_map;

    Map<GReg, GReg> coalesce_map;
    Map<GReg, GReg> hint_map;

    void try_allocate(int alloc_num);
    void try_split(int alloc_num);
    void spill(int alloc_num);
    PrioritizedAlloc get_alloc_priority(int alloc_num);

    Map<GReg, size_t> saved_registers;
    void add_saved_register(GReg reg);


};

} // namespace Backend
