#include "opt_2.h"

#include "ir_mem_instr.h"
#include "ir_ptr_instr.h"

namespace Opt2 {

bool BlockValue::operator == (const BlockValue &b)
{
    return uses == b.uses;
}

bool BlockValue::operator != (const BlockValue &b)
{
    return uses != b.uses;
}

void BlockValue::cup(const BlockValue &v)
{
    for(auto i : v.uses) {
        uses.insert(i);
    }
}

void BlockValue::clear()
{
    uses.clear();
}

void Utils::operator () (Ir::Block* p, BlockValue &v)
{
    /* printf("Calculate block %s\n", p->name());
    for(auto i : v.uses) {
        printf("    current use: %s\n", i.c_str());
    } */
    for(auto j=p->body.rbegin(); j!=p->body.rend(); ++j) {
        auto cur_instr = *j;
        /*
            1. %n = load %k -> use
            2. store %n, %k -> def
        */
        switch(cur_instr->instr_type()) {
            case Ir::INSTR_LOAD: {
                auto r = std::static_pointer_cast<Ir::LoadInstr>(cur_instr);
                v.uses.insert(r->operand(0)->usee->name());
                // printf("    LOCAL use = %s\n", r->operand(0)->usee->name());
                break;
            }
            case Ir::INSTR_STORE: {
                auto r = std::static_pointer_cast<Ir::StoreInstr>(cur_instr);
                // judge whether operand is from GEP
                auto to = r->operand(0)->usee;
                if(to->type() == Ir::VAL_INSTR) {
                    auto to_instr = static_cast<Ir::Instr*>(to);
                    if(to_instr->instr_type() != Ir::INSTR_ALLOCA) {
                        break; // shouldn't be killed
                    }
                }
                v.uses.erase(to->name());
                // printf("    LOCAL def = %s\n", r->operand(0)->usee->name());
                break;
            }
            case Ir::INSTR_ITEM: {
                auto r = std::static_pointer_cast<Ir::ItemInstr>(cur_instr);
                v.uses.insert(r->operand(0)->usee->name());
                break;
            }
            default:
                break;
        }
    }
}

int Utils::operator () (Ir::Block* p, const BlockValue &IN, const BlockValue &OUT)
{
    Set<String> uses = OUT.uses;
    int ans = 0;
    /* printf("In block %s\n", p->name());
    for(auto i : uses) {
        printf("    current use: %s\n", i.c_str());
    } */
    p->body = Ir::Instrs(p->body.rbegin(), p->body.rend());
    for(auto j=p->body.begin(); j!=p->body.end(); ) {
        auto cur_instr = *j;
        // printf("    at %s\n", cur_instr->instr_print());
        /*
            1. %n = load %k -> use
            2. store %n, %k -> def
        */
        switch(cur_instr->instr_type()) {
            case Ir::INSTR_LOAD: {
                auto r = std::static_pointer_cast<Ir::LoadInstr>(cur_instr);
                auto name = r->operand(0)->usee->name();
                uses.insert(name);
                // printf("    LOCAL use = %s\n", r->instr_print());
                break;
            }
            case Ir::INSTR_STORE: {
                auto r = std::static_pointer_cast<Ir::StoreInstr>(cur_instr);
                auto name = r->operand(0)->usee->name();
                auto to = r->operand(0)->usee;
                if(to->type() == Ir::VAL_INSTR) {
                    auto to_instr = static_cast<Ir::Instr*>(to);
                    if(to_instr->instr_type() != Ir::INSTR_ALLOCA) {
                        break; // shouldn't be killed
                    }
                }
                if(!uses.count(name)) { // this def is useless
                    // printf("    Not used def %s\n", r->instr_print());
                    j = p->body.erase(j);
                    ++ans;
                    goto End;
                } else {
                    // printf("    Def %s is used\n", r->instr_print());
                    uses.erase(name);
                }
                break;
            }
            default:
                break;
        }
        ++j;
End:
        continue;
    }
    p->body = Ir::Instrs(p->body.rbegin(), p->body.rend());
    return ans;
}

} // namespace Optimize
