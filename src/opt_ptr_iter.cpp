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
    my_assert(pred_block, "must exists");
    Ir::Block* succ_block = nullptr;
    Ir::Block* entry_block = nullptr;
    for (auto out_blk : loop->header->out_blocks()) {
        if (loop->loop_blocks.count(out_blk) == 0) {
            assert(!succ_block);
            succ_block = out_blk;
        } else if (out_blk != loop->header) {
            if (entry_block) return {}; // && or || is in condition
            entry_block = out_blk;
        }
    }
    // my_assert(succ_block, "must exists"); // may not exists
    return {{loop, phi, initial, from, entry_block, pred_block, succ_block, std::move(iterations)}};
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

constexpr int UNROLLING_CYCLE = 2;

bool loop_unrolling(Ir::BlockedProgram &func, const IterationInfo& info) {
    if (!info.succ_block) return false;
    size_t size = 0;
    for (auto&& block : info.loop->loop_blocks) {
        size += block->size();
        if (block == info.loop->header) continue;
        for (auto&& out : block->out_blocks()) {
            if (!info.loop->loop_blocks.count(out)) {
                return false;
            }
        }
    }
    // if (size > 100) return false; // too big
    if (info.loop->cmp_op != Ir::CMP_SLT) return false; // maybe CMP_SLE is also OK
    for (auto [_, increment] : info.iterations) {
        if (increment.step->name() != "1") return false;
    }
    fputs("unrolling...\n", stderr);
    // step 1   duplicate the loop
    // step 1.1 clone blocks
    Ir::CloneContext ctx;
    std::vector<Ir::pBlock> blocks;
    std::vector<Ir::pBlock> body;
    std::vector<Ir::pBlock> continues;
    Ir::Block* cloned_header = nullptr;
    int entry_index = -1; int index = entry_index;
    for (auto&& block : info.loop->loop_blocks) {
        blocks.push_back(block->clone(ctx));
        if (block == info.loop->header) {
            cloned_header = blocks.back().get();
        } else {
            ++index;
            if (block == info.entry_block) {
                assert(entry_index < 0);
                entry_index = index;
            }
            body.push_back(blocks.back());
            for (auto&& out : block->out_blocks()) {
                if (out == info.loop->header) {
                    continues.push_back(blocks.back());
                }
            }
        }
    }
    // if (continues.size() > 1) return false;
    assert(cloned_header);
    assert(entry_index >= 0);
    for (auto&& block : blocks) {
        block->fix_clone(ctx);
    }
    {
        auto inserter = func.find(info.loop->header);
        for (auto&& block : blocks) {
            func.insert(inserter, block);
        }
    }
    // step 1.2 reorganize loops
    info.pred_block->replace_out(info.loop->header, cloned_header);
    info.loop->header->replace_in(info.pred_block, cloned_header);
    cloned_header->replace_out(info.succ_block, info.loop->header);
    // info.succ_block->replace_in(cloned_header, info.loop->header); // leave this alone
    for (auto it1 = info.loop->header->begin(), it2 = cloned_header->begin(); it2 != cloned_header->end(); ++it1, ++it2) {
        auto phi1 = dynamic_cast<Ir::PhiInstr*>(it1->get());
        auto phi2 = dynamic_cast<Ir::PhiInstr*>(it2->get());
        if (phi1 && phi2) {
            for (size_t i = 0; i < phi1->phi_pairs(); ++i) {
                if (phi1->phi_val(i) == phi2->phi_val(i)) {
                    phi1->change_phi_val(i, phi2);
                }
            }
        }
    }
    // step 2   unrolling first loop
    // step 2.1 manipulate condition
    for (auto it = cloned_header->begin(); it != cloned_header->end(); ++it) {
        if (auto cmp = dynamic_cast<Ir::CmpInstr*>(it->get())) {
            auto add = Ir::make_binary_instr(Ir::INSTR_ADD, cmp->operand(0)->usee,
                func.add_imm(Value(ImmValue(UNROLLING_CYCLE - 1))).get());
            auto cmp2 = Ir::make_cmp_instr(cmp->cmp_type, add.get(), cmp->operand(1)->usee);
            cloned_header->insert(it, add);
            cloned_header->insert(it, cmp2);
            cmp->replace_self(cmp2.get());
            cloned_header->erase(it);
            break;
        }
    }
    // step 2.2 duplicate loop body

    auto initial_body = body;
    for (int cycle = 1; cycle < UNROLLING_CYCLE; ++cycle) {
        Ir::CloneContext body_ctx;
        std::vector<Ir::pInstr> sub_phis;
        Set<Ir::LabelInstr*> labels;
        for (auto&& block : blocks) {
            labels.insert(block->label().get());
        }
        for (auto&& instr : *cloned_header) {
            auto phi = dynamic_cast<Ir::PhiInstr*>(instr.get());
            if (!phi) continue;
            std::vector<std::pair<Ir::LabelInstr*, Ir::Val*>> incoming;
            for (auto [label, val] : *phi) {
                if (labels.count(label)) {
                    incoming.emplace_back(label, val);
                }
            }
            if (incoming.size() == 1) {
                body_ctx.map(phi, incoming[0].second);
            } else if (incoming.size() > 1) {
                auto sub_phi = Ir::make_phi_instr(phi->ty);
                for (auto [label, val] : incoming) {
                    sub_phi->add_incoming(label, val);
                }
                body_ctx.map(phi, sub_phi.get());
                sub_phis.push_back(sub_phi);
            }
        }
        std::vector<Ir::pBlock> cloned_body;
        std::vector<Ir::pBlock> cloned_continues;
        Map<Ir::PhiInstr*, Ir::Val*> cloned_phi_map;
        for (auto&& block : initial_body) {
            cloned_body.push_back(block->clone(body_ctx));
            for (auto&& out : block->out_blocks()) {
                if (out == cloned_header) {
                    cloned_continues.push_back(cloned_body.back());
                }
            }
        }
        for (auto&& block : cloned_body) {
            block->fix_clone(body_ctx);
        }
        auto inserter = ++func.find(body.back().get());
        for (auto&& block : cloned_body) {
            func.insert(inserter, block);
        }
        auto entry_block = cloned_body[entry_index];
        for (auto&& phi : sub_phis) {
            entry_block->push_after_label(phi);
        }
        for (auto&& cont : continues) {
            cont->replace_out(cloned_header, entry_block.get());
            // cloned_header->replace_in(cont.get(), static_cast<Ir::LabelInstr*>(body_ctx.lookup(cont->label().get()))->block());
        }
        for (auto&& instr : *cloned_header) {
            auto phi = dynamic_cast<Ir::PhiInstr*>(instr.get());
            if (!phi) continue;
            for (size_t i = 0; i < phi->phi_pairs(); ++i) {
                auto old_label = phi->phi_label(i);
                auto mapped_label = body_ctx.lookup(old_label);
                if (mapped_label != old_label) {
                    phi->change_phi_label(i, static_cast<Ir::LabelInstr *>(mapped_label));
                }
                auto old_val = phi->phi_val(i);
                auto mapped_val = body_ctx.lookup(old_val);
                if (old_val != mapped_val) {
                    phi->change_phi_val(i, mapped_val);
                }
            }
        }
        // for (auto&& block : body) {
        //     for (auto&& instr : *block) {
        //         auto phi = dynamic_cast<Ir::PhiInstr*>(instr.get());
        //         if (!phi) continue;
        //         for (auto [label, val] : *phi) {
        //             if (info.loop->loop_blocks.count(label->block())) {
        //                 cloned_phi_map[phi] = val;
        //                 break; // assume there is only one case
        //             }
        //         }
        //     }
        // }
        body = std::move(cloned_body);
        continues = std::move(cloned_continues);
    }
    return true;
}


void loop_unrolling(Ir::BlockedProgram &func, Alys::DomTree &dom) {
    Alys::LoopInfo loop_info(func, dom);
    for (auto&& [_, loop] : loop_info.loops) {
        if (auto info = detect_iteration(loop)) {
            if (loop_unrolling(func, info.value())) {
                break;
            }
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