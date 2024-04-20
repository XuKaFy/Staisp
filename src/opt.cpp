#include "opt.h"

namespace Optimize {

template<typename BlockValue, typename Utils>
void may_analysis(Ir::BlockedProgram &p);

template<typename BlockValue, typename Utils>
void must_analysis(Ir::BlockedProgram &p);

void optimize(Ir::pModule mod)
{
    for(auto i : mod->funsDefined) {
        Ir::BlockedProgram p;
        p.from_instrs(i->body);
        for(size_t cnt = 0; cnt < 1; ++cnt) {
            may_analysis<Opt1::BlockValue, Opt1::Utils>(p);
            remove_empty_block(p);
        }
        remove_empty_block(p);
        i->body = p.re_generate();
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
    List<Ir::pBlock> blist;
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Utils util;
    for(auto i : p.blocks) {
        blist.push_back(i);
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
    }
    while(blist.size()) {
        Ir::pBlock b = blist.front();
        blist.pop_front();

        BlockValue old_OUT = OUTs[b.get()];
        BlockValue& IN = INs[b.get()];
        BlockValue& OUT = OUTs[b.get()];

        IN.clear();
        for(auto i : b->in_block) {
            IN.cup(OUTs[i]);
        }

        OUT = IN;
        util(b, OUT); // transfer function
     
        if(old_OUT != OUT) {
            for(auto i : b->out_block) {
                blist.push_back(Ir::pBlock(i));
            }
        }
    }
}

template<typename BlockValue, typename Utils>
void must_analysis(Ir::BlockedProgram &p)
{
    List<Ir::pBlock> blist;
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Utils util;
    for(auto i : p.blocks) {
        blist.push_back(i);
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
    }
    while(blist.size()) {
        Ir::pBlock b = blist.front();
        blist.pop_front();

        BlockValue old_OUT = OUTs[b.get()];
        BlockValue& IN = INs[b.get()];
        BlockValue& OUT = OUTs[b.get()];

        IN.clear();
        for(auto i : b->in_block) {
            IN.cap(OUTs[i]);
        }

        OUT = IN;
        util.kill(b, OUT);
        util.gen(b, OUT);
     
        if(old_OUT != OUT) {
            for(auto i : b->out_block) {
                blist.push_back(Ir::pBlock(i));
            }
        }
    }
}

} // namespace Optimize
