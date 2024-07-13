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
    std::vector<StackObject> objects;
};

struct Func {
    Ir::pFuncDefined ir_func;
    std::string name;
    pFunctionType type;

    std::vector<Block> blocks;

    explicit Func(Ir::pFuncDefined ir_func)
        : ir_func(std::move(ir_func)) {
        name = ir_func->name();
        type = ir_func->function_type();
    }

// passes:
    void translate();
    Block translate(const Ir::pBlock &block);
    MachineInstrs translate(const Ir::pInstr &instr);
    bool peephole();
    void allocate_register();
    void save_register();
    Block generate_prolog() const;
    Block generate_epilog() const;
    String generate_asm() const;

// helper:
    int next_reg_{32};
    int next_reg() {
        return next_reg_++;
    }

    int excess_arguments = 0;
    int local_variables = 16; // for ra and fp
    int calculate_sp() const {
        int sp = local_variables + excess_arguments;
        if (sp % 8 != 0) sp += 8 - sp % 8;
        return sp;
    }
//
// todo: replace code above with below
//
    std::vector<StackObject> stack_objects;
    size_t stack_offset = 0;
    size_t alloc_stack(size_t size) {
        size_t id = stack_objects.size();
        stack_objects.push_back({stack_offset, size});
        stack_offset += size;
        return id;
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
