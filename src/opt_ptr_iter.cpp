#include "def.h"
#include "ir_block.h"
#include <iostream>
#include <ir_cast_instr.h>
#include <trans_loop.h>

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

std::optional<IterationInfo> detect_iteration(const Alys::pNaturalLoopBody& loop) {
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
            if (val->users().size() != 1 || val->users().at(0)->user != phi) {
                return {};
            }
            iterations.emplace(val, IterationInfo::SelfIncrement {step, label, negative});
        }
    }
    assert (initial); // no initial value? fancy case!
    assert (from);
    assert (!iterations.empty()); // no iteration? fancy case!
    Ir::Block* pred_block = nullptr;
    for (auto in_blk : loop->header->in_blocks()) {
        if (loop->loop_blocks.count(in_blk) == 0) {
            pred_block = in_blk;
            break;
        }
    }
    Ir::Block* succ_block = nullptr;
    for (auto out_blk : loop->header->out_blocks()) {
        if (loop->loop_blocks.count(out_blk) == 0) {
            succ_block = out_blk;
            break;
        }
    }
    my_assert(pred_block, "must exists");
    my_assert(succ_block, "must exists");
    return {{loop, phi, initial, from, pred_block, succ_block, std::move(iterations)}};
}

std::optional<IterationGEPInfo> detect_gep_iteration(IterationInfo info) {
    std::unordered_map<Ir::Val*, std::vector<Ir::MiniGepInstr*>> geps;
    for (auto&& use : info.phi->users()) {
        auto user = use->user;
        if (auto gep = dynamic_cast<Ir::MiniGepInstr*>(user)) {
            // OK if used by GEP
            auto array = gep->operand(0)->usee;
            if (auto instr = dynamic_cast<Ir::Instr*>(array)) {
                for (auto&& block : info.loop->loop_blocks) {
                    // but reject array generated inside loop
                    if (instr->block() == block) return {};
                }
            }
            geps[array].push_back(gep);
        } else if (info.iterations.count(user)) {
            // OK if used by iteration (as increment only, see above)
        } else if (auto instr = dynamic_cast<Ir::Instr*>(user); instr && instr->block() == info.loop->header) {
            // OK if used by condition (assumed that header contains only compare and branch)
        } else {
            // otherwise, it is forbidden
            return {};
        }
    }
    if (geps.empty()) return {};
    return {{std::move(info), std::move(geps)}};
}

void transform(const IterationGEPInfo& gepInfo) {
    auto [info, geps] = gepInfo;
    // step 1: construction new instructions
    std::vector<Ir::pInstr> bases, bounds, phis;
    Ir::pInstr cmp = nullptr;
    for (auto&& [array, geps] : geps) {
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
            cmp = Ir::make_cmp_instr(info.loop->cmp_op, phi.get(), bound.get());
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
        auto pred_blk = info.pred_block;
        for (auto&& base : bases) {
            pred_blk->push_behind_end(base);
        }
        for (auto&& bound : bounds) {
            pred_blk->push_behind_end(bound);
        }
        for (auto&& phi : phis) {
            block->push_behind_end(phi);
        }
        block->push_behind_end(cmp);
    }
    for (auto&& [original, increment]: info.iterations) {
        auto [step, from, neg/* no negative support now */ ] = increment;
        from->block()->erase(dynamic_cast<Ir::Instr*>(original));
    }
}

[[deprecated]] // usable but useless
void detect_sum_and_transform(IterationInfo info) {
    if (info.loop->loop_blocks.size() != 2) return;
    auto blocks = info.loop->loop_blocks;
    auto header = info.loop->header;
    blocks.erase(header);
    auto block = *blocks.begin();
    if (info.iterations.size() != 1) return;
    auto [iterated, increment] = *info.iterations.begin();
    if (increment.step->name() != "1" || info.loop->cmp_op != Ir::CMP_SLT) return;
    for (auto&& instr : *block) {
        if (instr->instr_type() == Ir::INSTR_LABEL || instr->is_terminator()) continue;
        if (instr->instr_type() != Ir::INSTR_BINARY) return;
        auto bin = dynamic_cast<Ir::BinInstr*>(instr.get());
        if (!dynamic_cast<Ir::PhiInstr*>(instr->operand(0)->usee))
            return;
        if (instr->operand(0)->usee == instr->operand(1)->usee)
            return;
        if (bin->binType == Ir::INSTR_ADD || bin->binType == Ir::INSTR_SUB) {
            if (block->contains(dynamic_cast<Ir::Instr*>(instr->operand(1)->usee)))
                return;
        } else {
            return;
        }
    }
    std::vector<Ir::PhiInstr*> phis;
    for (auto&& instr : *block) {
        auto bin = dynamic_cast<Ir::BinInstr*>(instr.get());
        if (!bin) continue;
        auto phi = dynamic_cast<Ir::PhiInstr*>(instr->operand(0)->usee);
        phis.push_back(phi);
        auto step = instr->operand(1)->usee;
        Ir::Val* initial = nullptr;
        for (auto [label, val] : *phi) {
            if (!info.loop->loop_blocks.count(label->block())) {
                assert(!initial);
                initial = val;
            }
        }
        assert(initial);
        auto times = Ir::make_binary_instr(Ir::INSTR_SUB, info.loop->bound, info.initial);
        header->push_behind_end(times);
        if (bin->binType == Ir::INSTR_ADD || bin->binType == Ir::INSTR_SUB) {
            auto mul = Ir::make_binary_instr(Ir::INSTR_MUL, times.get(), step);
            auto add = Ir::make_binary_instr(bin->binType, initial, mul.get());
            header->push_behind_end(mul);
            header->push_behind_end(add);
            phi->replace_self(add.get());
        }
    }
    header->squeeze_out(false);
    for (auto it = header->begin(); it != header->end(); ) {
        auto type = (*it)->instr_type();
        if (type == Ir::INSTR_CMP) {
            it = header->erase(it);
        } else {
            ++it;
        }
    }
    for (auto&& phi : phis) {
        header->erase(phi);
    }
}

void loop_unrolling(Ir::BlockedProgram &func, const IterationInfo& info) {
    Ir::CloneContext ctx;
    std::vector<Ir::pBlock> blocks;
    Ir::Block* cloned_header = nullptr;
    for (auto&& block : info.loop->loop_blocks) {
        blocks.push_back(block->clone(ctx));
        if (block == info.loop->header) {
            cloned_header = blocks.back().get();
        }
    }
    assert(cloned_header);
    for (auto&& block : blocks) {
        block->fix_clone(ctx);
    }
    for (auto&& block : blocks) {
        func.push_back(block);
    }
    info.pred_block->replace_out(info.loop->header, cloned_header);
    cloned_header->replace_out(info.succ_block, info.loop->header);
    info.loop->header->replace_in(info.pred_block, cloned_header);
    info.succ_block->replace_in(cloned_header, info.loop->header);
}


void loop_unrolling(Ir::BlockedProgram &func, Alys::DomTree &dom) {
    Alys::LoopInfo loop_info(func, dom);
    for (auto&& [_, loop] : loop_info.loops) {
        if (auto info = detect_iteration(loop)) {
            loop_unrolling(func, info.value());
            break;
        }
    }
}

void pointer_iteration(Ir::BlockedProgram &func, Alys::DomTree &dom) {
    Alys::LoopInfo loop_info(func, dom);
    for (auto&& [_, loop] : loop_info.loops) {
        if (auto info = detect_iteration(loop)) {
            if (auto gepInfo = detect_gep_iteration(info.value())) {
                transform(gepInfo.value());
            } else {
                // detect_sum_and_transform(info.value());
            }
        }
    }
}
}