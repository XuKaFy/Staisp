#include "opt_1.h"

namespace Opt1 {

bool BlockValue::operator == (const BlockValue &b)
{
    return val == b.val;
}

bool BlockValue::operator != (const BlockValue &b)
{
    return !((*this) == b);
}

void BlockValue::clear()
{
    val.clear();
}

void BlockValue::cup(const BlockValue &v)
{
    for(auto i : v.val) {
        if(!val.count(i.first)) {
            val[i.first] = i.second;
        } else {
            val.erase(i.first);
        }
    }
}

void Utils::operator () (Ir::pBlock p, BlockValue &v)
{
    for(auto i : p->body) {
        switch(i->instr_type()) {
        case Ir::INSTR_STORE: {
            auto r = std::static_pointer_cast<Ir::StoreInstr>(i);
            auto to = r->operand(0)->usee;
            auto val = r->operand(1)->usee;
            switch(val->type()) {
            case Ir::VAL_CONST: {
                auto con = std::static_pointer_cast<Ir::Const>(val);
                if(con->v.type() == VALUE_IMM) {
                    v.val[to->name()] = con->v.imm_value();
                } else {
                    v.val[to->name()] = Val(); // NAC
                }
                break;
            }
            case Ir::VAL_GLOBAL:
                v.val[to->name()] = Val(); // NAC
                break;
            case Ir::VAL_INSTR: {
                v.val[to->name()] = v.val[val->name()];
                break;
            }
            case Ir::VAL_BLOCK:
            case Ir::VAL_FUNC:
                throw Exception(1, "Utils", "Impossible Value in Block");
            }
            // printf("STORE %s, ty = %d\n", to->name(), v.val[to->name()].ty);
            break;
        }
        case Ir::INSTR_LOAD: {
            auto r = std::static_pointer_cast<Ir::LoadInstr>(i);
            auto from = r->operand(0)->usee;
            v.val[i->name()] = v.val[from->name()];
            // printf("LOAD %s, ty = %d\n", i->name(), v.val[i->name()].ty);
            break;
        }
        case Ir::INSTR_ALLOCA:
            break;
        case Ir::INSTR_BINARY:
        case Ir::INSTR_UNARY:
        case Ir::INSTR_CAST:
        case Ir::INSTR_CMP: {
            Vector<ImmValue> vv;
            for(size_t c=0; c<i->operand_size(); ++c) {
                auto oprd = i->operand(c)->usee;
                if(oprd->type() == Ir::VAL_INSTR) {
                    if(!v.val.count(oprd->name())) { // undef
                        v.val.erase(i->name()); // undef
                        goto End;
                    }
                    if(v.val[oprd->name()].ty == Val::NAC) {
                        v.val[i->name()] = Val();
                        goto End;
                    }
                } else {
                    if(std::static_pointer_cast<Ir::Const>(oprd)->v.type() != VALUE_IMM) {
                        v.val[i->name()] = Val();
                        goto End;    
                    }
                }
            }
            for(size_t c=0; c<i->operand_size(); ++c) {
                auto oprd = i->operand(c)->usee;
                if(oprd->type() == Ir::VAL_INSTR) {
                    vv.push_back(v.val[oprd->name()].v);
                } else {
                    vv.push_back(std::static_pointer_cast<Ir::Const>(oprd)->v.imm_value());
                }
            }
            v.val[i->name()] = std::static_pointer_cast<Ir::CalculatableInstr>(i)->calculate(vv);
End:
            // printf("CALCULATABLE %s, ty = %d\n", i->name(), v.val[i->name()].ty);
            break;
        }
        default:
            break;
        }
    }
    /*
    printf("In Block %s\n", p->name());
    for(auto i : v.val) {
        printf("    constant %s = %s\n", i.first.c_str(), i.second.v.print());
    }
    */
}

}; // namespace Opt1
