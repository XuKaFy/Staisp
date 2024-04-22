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
            // must_analysis<Opt2::BlockValue, Opt2::Utils>(i->p);
            remove_dead_code(i->p);
            remove_empty_block(i->p);
            i->p.join_blocks();
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

bool can_be_removed(Ir::InstrType t)
{
    switch(t) {
    case Ir::INSTR_RET:
    case Ir::INSTR_BR:
    case Ir::INSTR_BR_COND:
    case Ir::INSTR_STORE:
    case Ir::INSTR_ALLOCA:
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
    bool flag = false;
    for(auto i : p.blocks) {
        if(i->body.size() <= 1) continue;
        auto end = i->body.back();
        if(end->instr_type() == Ir::INSTR_BR_COND) {
            auto cond = end->operand(0)->usee;
            if(cond->type() == Ir::VAL_CONST) {
                auto con = static_cast<Ir::Const*>(cond);
                if(con->v.type() == VALUE_IMM) {
                    bool selected = (bool)con->v.imm_value();
                    // 0->trueTo
                    // 1->falseTo
                    auto new_br = std::static_pointer_cast<Ir::BrCondInstr>(end)->select(1-selected);
                    i->body.pop_back();
                    i->body.push_back(new_br);
                    flag = true;
                }
            }
        }
    }
    if(flag)
        p.generate_cfg(); // some br might be changed
}

template<typename BlockValue, typename Utils>
void from_top_analysis(Ir::BlockedProgram &p)
{
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Set<Ir::Block*> blist;
    Utils util;
    // blist.push_back(p.blocks.front().get());
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
    }
    while(blist.size()) {
        Ir::Block* b = *blist.begin();
        blist.erase(blist.begin());
        // printf("in block %s\n", b->name());

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
    Set<Ir::Block*> blist;
    Map<Ir::Block*, BlockValue> INs;
    Map<Ir::Block*, BlockValue> OUTs;
    Utils util;
    for(auto i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        blist.insert(i.get());
    }
    while(blist.size()) {
        Ir::Block* b = *blist.begin();
        blist.erase(blist.begin());

        BlockValue old_IN = INs[b];
        BlockValue& IN = INs[b];
        BlockValue& OUT = OUTs[b];

        OUT.clear();
        for(auto i : b->in_block) {
            OUT.cup(INs[i]);
        }

        IN = OUT;
        util(b, IN); // transfer function
     
        if(old_IN != IN) {
            for(auto i : b->out_block) {
                blist.insert(i);
            }
        }
    }
    for(auto i : p.blocks) {
        util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
}

} // namespace Optimize
