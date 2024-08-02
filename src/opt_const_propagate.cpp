#include "opt_const_propagate.h"

#include "ir_constant.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_phi_instr.h"
#include "ir_val.h"
#include "value.h"
#include <memory>
#include <optional>

namespace OptConstPropagate {

void ConstantMap::cup(const ConstantMap &map)
{
    // 1. undef + const = const
    // 2. undef + undef = undef
    // 3. const + const = { same: const; else: NAC }
    // 4. otherwise, NAC
    for (const auto &i : map.val) {
        if (!hasValue(i.first)) {
            // undef + const = const
            // undef + NAC = NAC
            val[i.first] = i.second;
            continue;
        }
        if (isValueNac(i.first)) {
            // NAC + * = NAC
            continue;
        }
        if (!i.second.has_value()) {
            // const + NAC = NAC
            val[i.first] = std::nullopt;
            continue;
        }
        if (value(i.first).value() != i.second.value()) {
            // const + const = else: NAC
            val[i.first] = std::nullopt;
            continue;
        }
        // const + const = same: const
    }
    /*
    for (auto i = val.begin(); i != val.end(); ) {
        if (i->second.has_value() && map.val.find(i->first) == map.val.end()) {
            // undef + const = undef
            i = val.erase(i);
            continue;
        }
        ++i;
    }
    */
}

void BlockValue::cup(const BlockValue &v) {
    val.cup(v.val);
}

void TransferFunction::operator()(Ir::Block *p, BlockValue &v) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
    printf("Analyzing Block %s\n", p->label()->name().c_str());
#endif
    for (const auto &i : *p) {
        switch (i->instr_type()) {
        case Ir::INSTR_PHI: {
            auto r = std::dynamic_pointer_cast<Ir::PhiInstr>(i);
            for (size_t i=0; i<r->phi_pairs(); ++i) {
                switch (r->phi_val(i)->type()) {
                case Ir::VAL_CONST: {
                    auto constant = dynamic_cast<Ir::Const*>(r->phi_val(i));
                    if (constant->v.type() == VALUE_IMM) {
                        v.val.setValue(r.get(), constant->v.imm_value());
                    } else {
                        v.val.setValueNac(r.get());
                        break;
                    }
                    break;
                }
                case Ir::VAL_INSTR: {
                    auto oprd_instr = dynamic_cast<Ir::Instr*>(r->phi_val(i));
                    if (!v.val.hasValue(oprd_instr)) {
                        v.val.setValueNac(r.get());
                        break;
                    }
                    v.val.transfer(r.get(), oprd_instr);
                    break;
                }
                default: {
                    v.val.setValueNac(r.get());
                    break;
                }
                } // end of switch
                if (v.val.isValueNac(r.get()))
                    break;
            }
            break;
        }
        case Ir::INSTR_STORE: {
            auto r = std::dynamic_pointer_cast<Ir::StoreInstr>(i);
            auto to = r->operand(0)->usee;
            auto val = r->operand(1)->usee;
            if (to->type() != Ir::VAL_INSTR) {
                // only for instr "alloca" or "gep"
                continue;
            }
            auto to_instr = dynamic_cast<Ir::Instr*>(to);
            if (to_instr->instr_type() != Ir::INSTR_ALLOCA) {
                // GEP might cause error
                break;
            }
            switch (val->type()) {
            case Ir::VAL_CONST: {
                auto con = static_cast<Ir::Const *>(val);
                if (con->v.type() == VALUE_IMM) {
                    v.val.setValue(to_instr, con->v.imm_value());
                } else {
                    v.val.setValueNac(to_instr);
                }
                break;
            }
            case Ir::VAL_GLOBAL:
                v.val.setValueNac(to_instr);
                break;
            case Ir::VAL_INSTR: {
                auto val_instr = dynamic_cast<Ir::Instr*>(val);
                if(val_instr->instr_type() == Ir::INSTR_SYM) {
                    // all symbols are NAK
                    v.val.setValueNac(to_instr);
                } else if (v.val.hasValue(val_instr)) {
                    v.val.transfer(to_instr, val_instr);
                }
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
            if (from->type() == Ir::VAL_GLOBAL) {
                auto global = static_cast<Ir::Global*>(from);
                if (global->is_effectively_final()) {
                    v.val.setValue(r.get(), global->con.v.imm_value());
                } else {
                    v.val.setValueNac(r.get());
                }
                continue;
            }
            if (from->type() == Ir::VAL_INSTR) {
                auto from_instr = dynamic_cast<Ir::Instr*>(from);
                if (from_instr->instr_type() == Ir::INSTR_ITEM) { // gep
                    v.val.setValueNac(r.get());
                } else if (v.val.hasValue(from_instr)) { // alloca
                    v.val.transfer(r.get(), from_instr);
                }
            }
            break;
        }
        case Ir::INSTR_CALL: {
            v.val.setValueNac(i.get()); // all function regarded as NAK
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
                if (oprd->type() == Ir::VAL_GLOBAL) {
                    v.val.setValueNac(i.get());
                    goto End;
                } else if (oprd->type() == Ir::VAL_INSTR) {
                    auto oprd_instr = dynamic_cast<Ir::Instr*>(oprd);
                    if (!v.val.hasValue(oprd_instr)) {  // undef
                        v.val.erase(i.get());
                        goto End;
                    }
                    if (v.val.isValueNac(oprd_instr)) { // not a constant
                        v.val.setValueNac(i.get());
                        goto End;
                    }
                } else {
                    if (static_cast<Ir::Const *>(oprd)->v.type() != VALUE_IMM) {
                        v.val.setValueNac(i.get());
                        goto End;
                    }
                }
            }
            for (size_t c = 0; c < i->operand_size(); ++c) {
                auto oprd = i->operand(c)->usee;
                if (oprd->type() == Ir::VAL_INSTR) {
                    vv.push_back(v.val.value(dynamic_cast<Ir::Instr*>(oprd)).value());
                } else {
                    vv.push_back(static_cast<Ir::Const *>(oprd)->v.imm_value());
                }
            }
            v.val.setValue(i.get(), std::dynamic_pointer_cast<Ir::CalculatableInstr>(i)->calculate(vv));
        End:
            break;
        }
        default:
            break;
        }
#ifdef OPT_CONST_PROPAGATE_DEBUG
        if (v.val.hasValue(i.get())) {
            if (v.val.isValueNac(i.get())) {
                printf("        GET VALUE [%s] NAK\n", i->instr_print().c_str());
            } else {
                printf("        GET VALUE [%s] = %s\n", i->instr_print().c_str(), v.val.value(i.get())->print().c_str());
            }
        }
#endif
    }
}

int TransferFunction::operator()(Ir::Block *p, const BlockValue &IN,
                      const BlockValue &OUT) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
    printf("TRANSFER FUNCTION IN BLOCK %s\n", p->label()->name().c_str());
#endif
    int ans = 0;
    OUT.val.ergodic([p, &ans](Ir::Instr* instr, ImmValue v) {
        if (instr->instr_type() == Ir::INSTR_ALLOCA)
            return ;
#ifdef OPT_CONST_PROPAGATE_DEBUG
        printf("    FOUND INSTR [%s] in Block %s\n", instr->name().c_str(), p->label()->name().c_str());
        printf("        [%s] TO [%s]\n", instr->instr_print().c_str(), v.print().c_str());
#endif
        auto imm = p->add_imm(v);
        instr->replace_self(imm.get());
        ++ans;
    });
    return ans;
}

}
