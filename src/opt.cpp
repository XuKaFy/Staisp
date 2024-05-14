#include "opt.h"

#include "opt_1.h"
#include "opt_2.h"

namespace Optimize {

template<typename BlockValue, typename Utils>
void from_top_analysis(Ir::BlockedProgram &p);

template<typename BlockValue, typename Utils>
void from_button_analysis(Ir::BlockedProgram &p);

void optimize(Ir::pModule mod)
{
    for(auto i : mod->funsDefined) {
        for(size_t cnt = 0; cnt < 5; ++cnt) {
            from_top_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
            i->p.normal_opt();
            from_button_analysis<Opt2::BlockValue, Opt2::Utils>(i->p);
            i->p.normal_opt();
        }
        i->p.re_generate();
    }
}

template<typename BlockValue, typename Utils>
void from_top_analysis(Ir::BlockedProgram &p)
{
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Set<Ir::Block*> blist;
    List<Ir::Block*> blist_seq;
    Utils util;
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while(blist.size()) {
        Ir::Block* b = *blist_seq.begin();
        blist_seq.pop_front();
        if(blist.count(b)) {
            blist.erase(b);
        } else {
            continue;
        }

        BlockValue old_OUT = OUTs[b];
        BlockValue& IN = INs[b];
        BlockValue& OUT = OUTs[b];

        IN.clear();
        for(auto i : b->in_block) {
            IN.cup(OUTs[i]);
        }

        OUT = IN;
        util(b, OUT); // transfer function
     
        if(old_OUT != OUT) {
            for(auto i : b->out_block) {
                blist.insert(i);
                blist_seq.push_back(i);
            }
        }
    }
    for(auto i : p.blocks) {
        util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
}

template<typename BlockValue, typename Utils>
void from_button_analysis(Ir::BlockedProgram &p)
{
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Set<Ir::Block*> blist;
    List<Ir::Block*> blist_seq;
    Utils util;
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
        blist_seq.push_back(i.get());
    }
    while(blist.size()) {
        Ir::Block* b = *blist_seq.begin();
        blist_seq.pop_front();
        if(blist.count(b)) {
            blist.erase(b);
        } else {
            continue;
        }

        BlockValue old_IN = INs[b];
        BlockValue& IN = INs[b];
        BlockValue& OUT = OUTs[b];

        OUT.clear();
        for(auto i : b->out_block) {
            OUT.cup(INs[i]);
        }

        IN = OUT;
        util(b, IN); // transfer function
     
        if(old_IN != IN) {
            // printf("Block %s changed\n", (b)->name());
            for(auto i : b->in_block) {
                blist.insert(i);
                blist_seq.push_back(i);
                // printf("    Updating %s\n", i->name());
            }
        }
    }
    for(auto i : p.blocks) {
        util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
}

} // namespace Optimize
