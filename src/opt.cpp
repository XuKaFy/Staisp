#include "opt.h"

#include "opt_1.h"
#include "opt_2.h"
#include "trans_SSA.h"

#include <iostream>
#define MAX_OPT_COUNT 10

namespace Optimize {

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p);

template <typename BlockValue, typename Utils>
int from_button_analysis(Ir::BlockedProgram &p);

void optimize(const Ir::pModule &mod) {
    for (auto &&i : mod->funsDefined) {
        i->p.normal_opt();
        //        i->p.re_generate();
    }
    for (auto &&i : mod->funsDefined) {
        size_t cnt = 0;
        SSA_pass pass(i->p, ssa_type::RECONSTRUCTION);
        std::cerr << i->print_func();
        pass.pass_transform();
        std::cerr << i->print_func();
        // i->print_func()
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = from_button_analysis<Opt2::BlockValue, Opt2::Utils>(i->p);
            i->p.normal_opt();
            opt_cnt += from_top_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
            i->p.normal_opt();
        }
        // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name().c_str(), cnt);
        i->p.re_generate();
    }
}

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    Set<Ir::Block *> blist;
    List<Ir::Block *> blist_seq;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while (!blist.empty()) {
        Ir::Block *b = *blist_seq.begin();
        blist_seq.pop_front();
        if (blist.count(b)) {
            blist.erase(b);
        } else {
            continue;
        }

        BlockValue old_OUT = OUTs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        auto in_block = b->in_blocks();
        IN.clear();
        if (!in_block.empty()) {
            IN = OUTs[*in_block.begin()];
            for (auto *i : in_block) {
                IN.cup(OUTs[i]);
            }
        }

        OUT = IN;
        util(b, OUT); // transfer function

        auto out_block = b->in_blocks();
        if (old_OUT != OUT) {
            for (auto *i : out_block) {
                blist.insert(i);
                blist_seq.push_back(i);
            }
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

template <typename BlockValue, typename Utils>
int from_button_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    Set<Ir::Block *> blist;
    List<Ir::Block *> blist_seq;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while (!blist.empty()) {
        Ir::Block *b = *blist_seq.begin();
        blist_seq.pop_front();
        if (blist.count(b)) {
            blist.erase(b);
        } else {
            continue;
        }

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks();
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto *i : out_block) {
                OUT.cup(INs[i]);
            }
        }

        IN = OUT;
        util(b, IN); // transfer function

        if (old_IN != IN) {
            // printf("Block %s changed\n", (b)->name());
            auto in_block = b->in_blocks();
            for (auto *i : in_block) {
                blist.insert(i);
                blist_seq.push_back(i);
                // printf("    Updating %s\n", i->name());
            }
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

} // namespace Optimize
