#include "opt_DSE.h"

#include "ir_mem_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"

namespace OptDSE {

bool BlockValue::operator==(const BlockValue &b) const {
    return uses == b.uses;
}

bool BlockValue::operator!=(const BlockValue &b) const {
    return uses != b.uses;
}

void BlockValue::cup(const BlockValue &v) {
    for (const auto &i : v.uses) {
        uses.insert(i);
    }
}

void BlockValue::clear() { uses.clear(); }

void TransferFunction::operator()(Ir::Block *p, BlockValue &v) {
    /* printf("Calculate block %s\n", p->name());
    for(auto i : v.uses) {
        printf("    current use: %s\n", i.c_str());
    } */
    for (auto j = p->rbegin(); j != p->rend(); ++j) {
        auto cur_instr = *j;
        /*
            1. %n = load %k -> use
            2. store %n, %k -> def
        */
        switch (cur_instr->instr_type()) {
        case Ir::INSTR_LOAD: {
            auto r = std::dynamic_pointer_cast<Ir::LoadInstr>(cur_instr);
            v.uses.insert(r->operand(0)->usee);
            // printf("    LOCAL use = %s\n", r->operand(0)->usee->name());
            break;
        }
        case Ir::INSTR_STORE: {
            auto r = std::dynamic_pointer_cast<Ir::StoreInstr>(cur_instr);
            // judge whether operand is from GEP
            auto to = r->operand(0)->usee;
            if (to->type() == Ir::VAL_INSTR) {
                auto to_instr = static_cast<Ir::Instr *>(to);
                if (to_instr->instr_type() != Ir::INSTR_ALLOCA) {
                    continue; // shouldn't be killed
                }
            }
            v.uses.erase(to);
            // printf("    LOCAL def = %s\n", r->operand(0)->usee->name());
            break;
        }
#ifdef USING_MINI_GEP
        case Ir::INSTR_MINI_GEP: {
            auto r = std::dynamic_pointer_cast<Ir::MiniGepInstr>(cur_instr);
            v.uses.insert(r->operand(0)->usee);
            break;
        }
#endif
        case Ir::INSTR_ITEM: {
            auto r = std::dynamic_pointer_cast<Ir::ItemInstr>(cur_instr);
            v.uses.insert(r->operand(0)->usee);
            break;
        }
        default:
            break;
        }
    }
}

int TransferFunction::operator()(Ir::Block *p, const BlockValue &IN,
                      const BlockValue &OUT) {
    Set<Ir::Val*> uses = OUT.uses;
    int ans = 0;
    for (auto j = p->rbegin(); j != p->rend();) {
        auto cur_instr = *j;
        /*
            1. %n = load %k -> use
            2. store %n, %k -> def
        */
        switch (cur_instr->instr_type()) {
        case Ir::INSTR_LOAD: {
            auto r = std::dynamic_pointer_cast<Ir::LoadInstr>(cur_instr);
            uses.insert(r->operand(0)->usee);
            break;
        }
        case Ir::INSTR_STORE: {
            auto r = std::dynamic_pointer_cast<Ir::StoreInstr>(cur_instr);
            auto to = r->operand(0)->usee;
            if (to->type() == Ir::VAL_INSTR) {
                auto to_instr = static_cast<Ir::Instr *>(to);
                if (to_instr->instr_type() != Ir::INSTR_ALLOCA) {
                    goto Ignore; // shouldn't be killed
                }
            }
            if (to->type() == Ir::VAL_GLOBAL) {
                goto Ignore;
            }
            if (uses.count(to) == 0U) {
                j = decltype(j)(p->erase(std::next(j).base()));
                ++ans;
                goto End;
            } else {
                uses.erase(to);
            }
            break;
        }
        default:
            break;
        }
    Ignore:
        ++j;
    End:
        continue;
    }
    return ans;
}

}
