#include "opt_1.h"

#include "ir_constant.h"
#include "ir_global.h"
#include "ir_mem_instr.h"
#include "ir_val.h"
#include <memory>

namespace Opt1 {

bool BlockValue::operator==(const BlockValue &b) const { return val == b.val; }

bool BlockValue::operator!=(const BlockValue &b) { return !((*this) == b); }

void BlockValue::clear() { val.clear(); }

void BlockValue::cup(const BlockValue &v) {
    Map<String, Val> copied_val;
    for (const auto &i : v.val) {
        if (val.count(i.first) == 0U) { // undef + constant = undef;
            continue;
        }
        if (i.second.ty == Val::NAC || val[i.first].ty == Val::NAC) {
            copied_val[i.first] = Val(); // one NAC, all NAC
            continue;
        }
        if (i.second != val[i.first]) { // different constant
            copied_val[i.first] = Val();
            continue;
        }
        copied_val[i.first] = i.second;
    }
    val = copied_val;
}

void Utils::operator()(Ir::Block *p, BlockValue &v) {
    for (const auto &i : *p) {
        switch (i->instr_type()) {
        case Ir::INSTR_STORE: {
            auto r = std::dynamic_pointer_cast<Ir::StoreInstr>(i);
            auto to = r->operand(0)->usee;
            auto val = r->operand(1)->usee;
            if (to->type() == Ir::VAL_GLOBAL) {
                // store to global val but we dont know its value after this
                // store
                v.val[to->name()] = Val();
                continue;
            }
            switch (val->type()) {
            case Ir::VAL_CONST: {
                auto con = static_cast<Ir::Const *>(val);
                if (con->v.type() == VALUE_IMM) {
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
            break;
        }
        case Ir::INSTR_LOAD: {
            auto r = std::dynamic_pointer_cast<Ir::LoadInstr>(i);
            auto from = r->operand(0)->usee;
            if (from->type() == Ir::VAL_GLOBAL &&
                !static_cast<Ir::Global *>(from)->is_const) {
                // all global val must be VAC, except const
                v.val[i->name()] = Val();
            } else {
                v.val[i->name()] = v.val[from->name()];
                v.val[i->name()].ir = i;
            }
            break;
        }
        case Ir::INSTR_ALLOCA:
            break;
        case Ir::INSTR_BINARY:
        case Ir::INSTR_UNARY:
        case Ir::INSTR_CAST:
        case Ir::INSTR_CMP: {
            Vector<ImmValue> vv;
            for (size_t c = 0; c < i->operand_size(); ++c) {
                auto oprd = i->operand(c)->usee;
                if (oprd->type() == Ir::VAL_INSTR) {
                    if (v.val.count(oprd->name()) == 0U) { // undef
                        v.val.erase(i->name());            // undef
                        goto End;
                    }
                    if (v.val[oprd->name()].ty == Val::NAC) {
                        v.val[i->name()] = Val();
                        goto End;
                    }
                } else {
                    if (static_cast<Ir::Const *>(oprd)->v.type() != VALUE_IMM) {
                        v.val[i->name()] = Val();
                        goto End;
                    }
                }
            }
            for (size_t c = 0; c < i->operand_size(); ++c) {
                auto oprd = i->operand(c)->usee;
                if (oprd->type() == Ir::VAL_INSTR) {
                    vv.push_back(v.val[oprd->name()].v);
                } else {
                    vv.push_back(static_cast<Ir::Const *>(oprd)->v.imm_value());
                }
            }
            v.val[i->name()] = Val(
                std::dynamic_pointer_cast<Ir::CalculatableInstr>(i)->calculate(
                    vv));
            v.val[i->name()].ir = i;
        End:
            break;
        }
        default:
            break;
        }
    }
}

int Utils::operator()(Ir::Block *p, const BlockValue &IN,
                      const BlockValue &OUT) {
    int ans = 0;
    for (const auto &i : OUT.val) {
        if (i.second.ty == Val::VALUE &&
            i.second.ir) { // has value, is a constant
            auto imm = Ir::make_constant(i.second.v);
            p->add_imm(imm);
            i.second.ir->replace_self(imm.get());
            ++ans;
        }
    }
    return ans;
}

}; // namespace Opt1
