#pragma once

#include <type.h>
#include <utility>

#include "bkd_block.h"
#include "def.h"

namespace Backend {

struct Func {
    std::string name;
    pFunctionType type;

    explicit Func(std::string name, pFunctionType type)
        : name(std::move(name)), type(std::move(type)) {}

    String print() const;

    Vector<Block> body;

    int live_register_analysis();
    void allocate_register();
    bool peephole();

    Block generate_prolog() const;
    Block generate_epilog() const;


    int excess_arguments = 0;
    int local_variables = 16; // for ra and fp
    // where are saved registers and overflowed registers?

    int calculate_sp() const {
        int sp = local_variables + excess_arguments;
        if (sp % 8 != 0) sp += 8 - sp % 8;
        return sp;
    }

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
