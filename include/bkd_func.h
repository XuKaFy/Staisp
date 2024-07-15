#pragma once

#include <ir_func.h>
#include <ir_func_defined.h>
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
spilled registers
local variables
caller saved registers
callee saved registers
spilled arguments

*/
struct StackFrame {
    StackObject ra{0, 8};
    size_t offset = 8;
    std::vector<StackObject> locals; // all except ra and args
    size_t push(size_t size) {
        size_t index = locals.size();
        locals.push_back({offset, size});
        offset += size;
        return index;
    }

    std::vector<StackObject> args;
    void at_least_args(size_t n) {
        if (n <= 8) return;
        n -= 8; // first eight arguments are in registers
        for (size_t i = args.size(); i < n; ++i) {
            // rest of arguments then are in stack
            args.push_back({i * 8, 8});
        }
    }
    // don't call this too early
    // and don't forget to call this
    void adjust_args() {
        for (auto& arg : args) {
            arg.offset += offset;
        }
    }

    size_t size() {
        size_t alignment;
        size_t sz = 8;
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
    bool peephole();
    void allocate_register();
    void save_register();
    void generate_prolog();
    void generate_epilog();
    String generate_asm() const;

    void passes() {
        translate();
        while (peephole()) {}
        allocate_register();
        save_register();
        generate_prolog();
        generate_epilog();
    }


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

    int live_register_analysis();
};

struct BlockValue {
    bool operator==(const BlockValue &b) const;
    bool operator!=(const BlockValue &b) const;

    void cup(const BlockValue &v);

    void clear();

    Set<Reg> uses;
    Set<FReg> fuses;
};

struct TransferFunction {
    void operator()(const Block *p, BlockValue &v);
    int operator()(const Block *p, const BlockValue &IN, const BlockValue &OUT);
};

// copied from DFA, for liveness analysis
int from_bottom_analysis(Func &p);

} // namespace Backend
