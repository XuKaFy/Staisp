#include "opt.h"

#include "opt_1.h"
#include "opt_2.h"
#include "trans_SSA.h"

#define MAX_OPT_COUNT 10

namespace Optimize {

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p);

template <typename BlockValue, typename Utils>
int from_bottom_analysis(Ir::BlockedProgram &p);

void optimize(const Ir::pModule &mod) {
    for (auto &&i : mod->funsDefined) {
        // i->inline_self();
        i->p.normal_opt();
        //        i->p.re_generate();
    }
    for (auto &&i : mod->funsDefined) {
        SSA_pass pass(i->p, ssa_type::RECONSTRUCTION);
        pass.pass_transform();
        // i->print_func()
        // for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
        //     opt_cnt = from_button_analysis<Opt2::BlockValue,
        //     Opt2::Utils>(i->p); i->p.normal_opt(); opt_cnt +=
        //     from_top_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
        //     i->p.normal_opt();
        // }
        // // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name().c_str(), cnt);
        i->p.re_generate();
    }
}

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = pending_blocks.front();
        pending_blocks.pop_front();

        BlockValue old_OUT = OUTs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        auto in_block = b->in_blocks();
        IN.clear();
        if (!in_block.empty()) {
            IN = OUTs[*in_block.begin()];
            for (auto block : in_block) {
                IN.cup(OUTs[block]);
            }
        }

        OUT = IN;
        util(b, OUT); // transfer function

        if (old_OUT != OUT) {
            auto out_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), out_block.begin(), out_block.end());
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

template <typename BlockValue, typename Utils>
int from_bottom_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = *pending_blocks.begin();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks();
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        util(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

} // namespace Optimize
