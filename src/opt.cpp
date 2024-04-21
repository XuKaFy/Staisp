#include "opt.h"

namespace Optimize {

template<typename BlockValue, typename Utils>
void may_analysis(Ir::BlockedProgram &p);

template<typename BlockValue, typename Utils>
void must_analysis(Ir::BlockedProgram &p);

void optimize(Ir::pModule mod)
{
    for(auto i : mod->funsDefined) {
        for(size_t cnt = 0; cnt < 1; ++cnt) {
            may_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
            remove_empty_block(i->p);
        }
        i->p.re_generate();
    }
}

void remove_empty_block(Ir::BlockedProgram &p)
{
    my_assert(p.blocks.size(), "?");
    for(auto i = p.blocks.begin() + 1; i != p.blocks.end();) {
        if((*i)->in_block.size() == 0) {
            i = p.blocks.erase(i);
        } else ++i;
    }
}

template<typename BlockValue, typename Utils>
void may_analysis(Ir::BlockedProgram &p)
{
    List<Ir::Block*> blist;
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Utils util;
    blist.push_back(p.blocks.front().get());
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
    }
    while(blist.size()) {
        Ir::Block* b = blist.front();
        blist.pop_front();

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
                blist.push_back(i);
            }
        }
    }
    for(auto i : p.blocks) {
        util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
}

template<typename BlockValue, typename Utils>
void must_analysis(Ir::BlockedProgram &p)
{
    List<Ir::Block*> blist;
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Utils util;
    blist.push_back(p.blocks.front().get());
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
    }
    while(blist.size()) {
        Ir::Block* b = blist.front();
        blist.pop_front();

        BlockValue old_OUT = OUTs[b];
        BlockValue& IN = INs[b];
        BlockValue& OUT = OUTs[b];

        IN.clear();
        for(auto i : b->in_block) {
            IN.cap(OUTs[i]);
        }

        OUT = IN;
        util(b, OUT); // transfer function
     
        if(old_OUT != OUT) {
            for(auto i : b->out_block) {
                blist.push_back(i);
            }
        }
    }
    for(auto i : p.blocks) {
        util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
}

} // namespace Optimize
