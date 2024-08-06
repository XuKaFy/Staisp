#include <iostream>
#include <trans_loop.h>

namespace Optimize {

struct IterationInfo {
    struct SelfIncrement {
        Ir::Val* step;
        Ir::LabelInstr* from;
        bool negative; // reserved for future use
    };
    Alys::pNaturalLoopBody loop;
    Ir::Val* initial;
    Ir::LabelInstr* from;
    // array -> GEPs
    std::unordered_map<Ir::Val*, std::vector<Ir::MiniGepInstr*>> geps;
    // iteration -> optional corresponding self-increment
    std::unordered_map<Ir::Val*, SelfIncrement> iterations;

    void print() {
        std::cerr << loop->print() << std::flush;
        std::cerr << "initial: " << initial->name() << std::endl;
        std::cerr << geps.size() << " arrays: ";
        for (auto&& [array, gep] : geps) {
            std::cerr << array->name() << " ";
        }
        std::cerr << std::endl << "iterations: " << std::endl;
        for (auto&& [iteration, increment] : iterations) {
            std::cerr << "    " << iteration->name() << " with self-increment: "
                      << (increment.negative ? '-' : '+') << increment.step->name() << std::endl;
        }
        std::cerr << std::endl;
    }
};

std::optional<IterationInfo> detect_iteration(Alys::pNaturalLoopBody loop) {
    auto phi = dynamic_cast<Ir::PhiInstr*>(loop->ind);
    if (phi == nullptr || is_float(phi->ty)) return {};
    Ir::Val* initial = nullptr;
    Ir::LabelInstr* from = nullptr;
    std::unordered_map<Ir::Val*, IterationInfo::SelfIncrement> iterations;
    for (auto [label, val] : *phi) {
        if (!loop->loop_blocks.count(label->block())) {
            // outside loop -> initial value
            assert (!initial); // multiple initial value? fancy case!
            initial = val;
            from = label;
        } else {
            // inside loop -> iteration
            Ir::Val* step = nullptr; bool negative = false;
            if (auto bin = dynamic_cast<Ir::BinInstr*>(val)) {
                auto lhs = bin->operand(0)->usee;
                auto rhs = bin->operand(1)->usee;
                if (bin->binType == Ir::INSTR_ADD) {
                    if (lhs == phi && rhs == phi) return {};
                    if (lhs == phi) {
                        step = rhs;
                    } else if (rhs == phi) {
                        step = lhs;
                    }
                }
                // no negative support now
                // else if (bin->binType == Ir::INSTR_SUB) {
                //     if (lhs == phi) {
                //         step = rhs;
                //         negative = true;
                //     }
                // }
            }
            if (step == nullptr) return {};
            iterations.emplace(val, IterationInfo::SelfIncrement {step, label, negative});
        }
    }
    assert (initial); // no initial value? fancy case!
    assert (from);
    assert (!iterations.empty()); // no iteration? fancy case!
    std::unordered_map<Ir::Val*, std::vector<Ir::MiniGepInstr*>> geps;
    for (auto&& use : phi->users) {
        auto user = use->user;
        if (auto gep = dynamic_cast<Ir::MiniGepInstr*>(user)) {
            // OK if used by GEP
            geps[gep->operand(0)->usee].push_back(gep);
        } else if (iterations.count(user)) {
            // OK if used by iteration (as increment only, see above)
        } else if (auto instr = dynamic_cast<Ir::Instr*>(user); instr && instr->block() == loop->header) {
            // OK if used by condition (assumed that header contains only compare and branch)
        } else {
            // otherwise, it is forbidden
            return {};
        }
    }
    if (geps.empty()) return {};
    return {{std::move(loop), initial, from, std::move(geps), std::move(iterations)}};
}

void transform(const IterationInfo& info) {
    // step 1: construction new instructions
    std::vector<Ir::pInstr> bases, bounds, phis;
    Ir::pInstr cmp = nullptr;
    for (auto&& [array, geps] : info.geps) {
        // I hope there is only one gep for every array
        // as long as GVN is implemented
        auto in_this_dim = geps.front()->in_this_dim;
        auto base = Ir::make_mini_gep_instr(array, info.initial, in_this_dim);
        auto bound = Ir::make_mini_gep_instr(array, info.loop->bound, in_this_dim);
        auto phi = Ir::make_phi_instr(base->ty);
        phi->add_incoming(info.from, base.get());
        for (auto&& [original, increment]: info.iterations) {
            auto [step, from, neg/* no negative support now */ ] = increment;
            auto iter = Ir::make_mini_gep_instr(phi.get(), step, true);
            phi->add_incoming(from, iter.get());
            auto block = from->block();
            block->push_behind_end(iter);
        }
        if (cmp == nullptr)
            cmp = Ir::make_cmp_instr(info.loop->cmp_op, phi, bound);
        for (auto& gep : geps) {
            gep->replace_self(phi.get());
            gep->block()->erase(gep);
        }
        bases.push_back(std::move(base));
        bounds.push_back(std::move(bound));
        phis.push_back(std::move(phi));
    }

    assert(cmp);
    {
        auto block = info.loop->header;
        auto original_phi = dynamic_cast<Ir::PhiInstr*>(info.loop->ind);
        info.loop->loop_cnd_instr->replace_self(cmp.get());
        block->erase(original_phi);
        block->erase(info.loop->loop_cnd_instr);
        for (auto&& base : bases) {
            block->push_behind_end(base);
        }
        for (auto&& bound : bounds) {
            block->push_behind_end(bound);
        }
        for (auto&& phi : phis) {
            block->push_behind_end(phi);
        }
        block->push_behind_end(cmp);
    }
    for (auto&& [original, increment]: info.iterations) {
        auto [step, from, neg/* no negative support now */ ] = increment;
        auto block = from->block();
        block->erase(dynamic_cast<Ir::Instr*>(original));
    }
}

void pointer_iteration(Ir::BlockedProgram &func, Alys::DomTree &dom) {
    Alys::LoopInfo loop_info(func, dom);
    for (auto&& [_, loop] : loop_info.loops) {
        if (auto info = detect_iteration(loop)) {
            info->print();
            transform(info.value());
        }
    }
}

}