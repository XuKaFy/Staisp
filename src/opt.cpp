#include "opt.h"

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
            remove_dead_code(i->p);
            remove_empty_block(i->p);
            i->p.join_blocks();
            // i->p.print_cfg();
            from_button_analysis<Opt2::BlockValue, Opt2::Utils>(i->p);
            remove_dead_code(i->p);
            remove_empty_block(i->p);
            i->p.join_blocks();
            // i->p.print_cfg();
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
    for(auto i = p.blocks.begin() + 1; i != p.blocks.end();) {
        if((*i)->out_block.size() == 1 && (*i)->body.size() == 2) {
            // printf("Remove Empty Br Block %s\n", (*i)->name());
            (*i)->connect_in_and_out();
            i = p.blocks.erase(i);
        } else ++i;
    }
}

bool can_be_removed(Ir::InstrType t)
{
    switch(t) {
    case Ir::INSTR_RET:
    case Ir::INSTR_BR:
    case Ir::INSTR_BR_COND:
    case Ir::INSTR_STORE:
    case Ir::INSTR_CALL:
        return false;
    default:
        return true;
    }
    return true;
}

void remove_dead_code(Ir::BlockedProgram &p)
{
    my_assert(p.blocks.size(), "?");
    for(auto i : p.blocks) {
        for(auto j = i->body.begin()+1; j!=i->body.end(); ) {
            if((*j)->users.empty() && can_be_removed((*j)->instr_type())) {
                j = i->body.erase(j);
            } else {
                ++j;
            }
        }
    }
    
    for(auto i : p.blocks) {
        if(i->body.size() <= 1) continue;
        
        auto end = i->body.back();
        if(end->instr_type() != Ir::INSTR_BR_COND) continue;

        auto cond = end->operand(0)->usee;
        if(cond->type() != Ir::VAL_CONST) continue;
        
        auto con = static_cast<Ir::Const*>(cond);
        if(con->v.type() == VALUE_IMM) {
            i->squeeze_out((bool) con);
        }
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
