#pragma once

#include "ir_block.h"
#include "alys_dom.h"
#include "trans_loop.h"

#include <iostream>

namespace Optimize {


struct IterationInfo {
    struct SelfIncrement {
        Ir::Val* step;
        Ir::LabelInstr* from;
        bool negative; // reserved for future use
    };
    Alys::pNaturalLoopBody loop;
    Ir::PhiInstr* phi;
    Ir::Val* initial;
    Ir::LabelInstr* from;
    Ir::Block* entry_block;
    Ir::Block* pred_block;
    Ir::Block* succ_block;
    // iteration -> optional corresponding self-increment
    std::unordered_map<Ir::Val*, SelfIncrement> iterations;

    void print() {
        std::cerr << loop->print() << std::flush;
        std::cerr << "initial: " << initial->name() << std::endl;
        std::cerr << std::endl << "iterations: " << std::endl;
        for (auto&& [iteration, increment] : iterations) {
            std::cerr << "    " << iteration->name() << " with self-increment: "
                      << (increment.negative ? '-' : '+') << increment.step->name() << std::endl;
        }
        std::cerr << std::endl;
    }
};

struct IterationGEPInfo {
    IterationInfo info;
    // array -> GEPs
    std::unordered_map<Ir::Val*, std::vector<Ir::MiniGepInstr*>> geps;

    void print() {
        std::cerr << geps.size() << " arrays: ";
        for (auto&& [array, gep] : geps) {
            std::cerr << array->name() << " ";
        }
    }

};

void pointer_iteration(Ir::BlockedProgram &func, Alys::DomTree &dom);
void loop_unrolling(Ir::BlockedProgram &func, Alys::DomTree &dom);
std::optional<IterationInfo> detect_iteration(const Alys::pNaturalLoopBody& loop);

} // namespace Optimize
