#include "opt.h"

#include "opt_1.h"
#include "opt_2.h"

#define MAX_OPT_COUNT 10

namespace Optimize {

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p);

template <typename BlockValue, typename Utils>
int from_button_analysis(Ir::BlockedProgram &p);

void optimize(Ir::pModule mod) {
    for (auto &&i : mod->funsDefined) {
        i->p.normal_opt();
    }
    // printf("%s\n", mod->print_module().c_str());
    for (auto &&i : mod->funsDefined) {
        size_t cnt = 0;
        // printf("Optimizing function %s\n", i->name().c_str());
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && opt_cnt; ++cnt) {
            // printf("  Loop: %lu\n", cnt);
            opt_cnt = from_button_analysis<Opt2::BlockValue, Opt2::Utils>(i->p);
            i->p.normal_opt();
            opt_cnt += from_top_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
            i->p.normal_opt();
            // printf("%s\n", mod->print_module().c_str());
        }
        // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name(), cnt);
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
    for (auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while (blist.size()) {
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

        IN.clear();
        for (auto i : b->in_block) {
            IN.cup(OUTs[i]);
        }

        OUT = IN;
        util(b, OUT); // transfer function

        if (old_OUT != OUT) {
            for (auto i : b->out_block) {
                blist.insert(i);
                blist_seq.push_back(i);
            }
        }
    }
    for (auto i : p.blocks) {
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
    for (auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while (blist.size()) {
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
        for (auto i : b->out_block) {
            OUT.cup(INs[i]);
        }

        IN = OUT;
        util(b, IN); // transfer function

        if (old_IN != IN) {
            // printf("Block %s changed\n", (b)->name());
            for (auto i : b->in_block) {
                blist.insert(i);
                blist_seq.push_back(i);
                // printf("    Updating %s\n", i->name());
            }
        }
    }
    for (auto i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

} // namespace Optimize
