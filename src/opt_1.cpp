#include "opt_1.h"

#include "ir_mem_instr.h"
#include "ir_constant.h"

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
            if(i.second.ty == Val::NAC || val[i.first].ty == Val::NAC) {
                val[i.first] = Val(); // one NAC, all NAC
            } else {
                if(i.second != val[i.first]) { // different constant
                    val[i.first] = Val();
                }
            }
        }
    }
}

void Utils::operator () (Ir::Block* p, BlockValue &v)
{
    // printf("Start Block %s\n", p->name());
    for(auto i : p->body) {
        switch(i->instr_type()) {
        case Ir::INSTR_STORE: {
            auto r = std::static_pointer_cast<Ir::StoreInstr>(i);
            auto to = r->operand(0)->usee;
            auto val = r->operand(1)->usee;
            switch(val->type()) {
            case Ir::VAL_CONST: {
                auto con = static_cast<Ir::Const*>(val);
                if(con->v.type() == VALUE_IMM) {
                    v.val[to->name()] = Val(con->v.imm_value());
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
            v.val[i->name()].ir = i;
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
                    if(static_cast<Ir::Const*>(oprd)->v.type() != VALUE_IMM) {
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
                    vv.push_back(static_cast<Ir::Const*>(oprd)->v.imm_value());
                }
            }
            v.val[i->name()] = Val(std::static_pointer_cast<Ir::CalculatableInstr>(i)->calculate(vv));
            v.val[i->name()].ir = i;
End:
            // printf("CALCULATABLE %s, ty = %d\n", i->name(), v.val[i->name()].ty);
            break;
        }
        default:
            break;
        }
    }
    /* printf("In Block %s\n", p->name());
    for(auto i : v.val) {
        if(i.second.ty == Val::VALUE && i.second.ir) { // has value, is a constant
            // printf("    instr: %s\n", i.second.ir->instr_print());
            printf("    constant %s = %s\n", i.first.c_str(), i.second.v.print());
        }
    } */
}

int Utils::operator () (Ir::Block* p, const BlockValue &IN, const BlockValue &OUT)
{
    int ans = 0;
    // printf("In Block %s\n", p->name());
    for(auto i : OUT.val) {
        if(i.second.ty == Val::VALUE && i.second.ir) { // has value, is a constant
            // printf("    instr: %s\n", i.second.ir->instr_print());
            // printf("    constant %s = %s\n", i.first.c_str(), i.second.v.print());
            auto imm = Ir::make_constant(i.second.v);
            p->add_imm(imm);
            i.second.ir->replace_self(imm.get());
            ++ans;
        }
    }
    return ans;
}

}; // namespace Opt1
