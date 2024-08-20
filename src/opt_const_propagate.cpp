#include "opt_const_propagate.h"

#include "imm.h"
#include "ir_constant.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_phi_instr.h"
#include "ir_call_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"
#include "type.h"
#include "ir_cast_instr.h"
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
}

struct ValResult {
    enum State {
        UNDEF,
        VALUE,
        NAC,
    } stat;
    ImmValue val;
    
    ValResult(State t = UNDEF)
        : stat(t) { }
    ValResult(const ImmValue &val)
        : stat(VALUE), val(val){ }
};

void fill_val(Ir::Instr* instr, const ValResult &res, BlockValue &v)
{
    // printf("*** FILL %s with stat = %s, val = %s\n", instr->instr_print().c_str(), res.stat == ValResult::NAC ? "NAC" : (res.stat == ValResult::VALUE ? "VALUE" : "UNDEF"), res.val.print().c_str());
    switch (res.stat) {
    case ValResult::NAC:
        v.val.setValueNac(instr);
        break;
    case ValResult::UNDEF:
        break;
    case ValResult::VALUE:
        v.val.setValue(instr, res.val);
        break;
    }
}

ValResult read_val(Ir::Val* val, BlockValue &v) {
    switch (val->type()) {
    case Ir::VAL_CONST: {
        auto constant = dynamic_cast<Ir::Const*>(val);
        if (constant->v.type() == VALUE_IMM) {
            return constant->v.imm_value();
        } else {
            return ValResult::NAC;
        }
        break;
    }
    case Ir::VAL_INSTR: {
        auto oprd_instr = dynamic_cast<Ir::Instr*>(val);
        /*if (oprd_instr->instr_type() == Ir::INSTR_MINI_GEP) {
            // when loading a GEP
            // first check whether its array is disabled
            auto arr = dynamic_cast<Ir::AllocInstr*>(oprd_instr->operand(0)->usee);
            if (arr == nullptr) {
                return ValResult::NAC;
            }
            if (v.val.hasValue(arr) && v.val.isValueNac(arr)) {
                return ValResult::NAC;
            }
                
            if (oprd_instr->operand(1)->usee->type() != Ir::VAL_CONST) {
                return ValResult::NAC;
            }
    
            int64_t index = static_cast<Ir::Const*>(oprd_instr->operand(1)->usee)->v.imm_value().val.ival;
            auto res = v.val.getArrayValue(arr, index);
            if (res.has_value()) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
                printf("    LOAD %s[%ld] = %s \n", arr->name().c_str(), index, res->print().c_str());
#endif

                return res.value();
            }
            return ValResult::UNDEF;
        }*/
        if (v.val.hasValue(oprd_instr)) {
            auto res = v.val.value(oprd_instr);
            if (res) {
                return res.value();
            }
            return ValResult::NAC;
        }
        if (oprd_instr->instr_type() == Ir::INSTR_SYM) {
            // all symbol is NAC
            return ValResult::NAC;
        }
        if (oprd_instr->instr_type() == Ir::INSTR_MINI_GEP) {
            // for situation that first use
            return ValResult::NAC;
        }
        return ValResult::UNDEF;
    }
    case Ir::VAL_GLOBAL: {
        /* auto global = static_cast<Ir::Global*>(val);
        if (global->is_effectively_final() && is_basic_type(global->con.v.ty)) {
            return global->con.v.imm_value();
        } */
        return ValResult::NAC;
    }
    default: break;
    }
    return ValResult::NAC;
}

void when_store(Ir::StoreInstr* store, BlockValue& v)
{
    auto to = store->operand(0)->usee;
    auto val = store->operand(1)->usee;
    
    if (to->type() != Ir::VAL_INSTR) {
        // only for instr "alloca" or "gep"
        return ;
    }

    auto to_instr = dynamic_cast<Ir::Instr*>(to);
    
    if (to_instr->instr_type() == Ir::INSTR_ALLOCA) {
        fill_val(to_instr, read_val(val, v), v);
        return ;
    }

    if(to_instr->instr_type() == Ir::INSTR_MINI_GEP) {
        auto target = to_instr->operand(0)->usee;
        auto index = to_instr->operand(1)->usee;
        
        // only for (instr)[i]
        if (target->type() != Ir::VAL_INSTR) {
            return ;
        }
        // only for (1-dim-array / alloca [N x i32])[i]
        auto arr_target = dynamic_cast<Ir::AllocInstr*>(target);
        if (arr_target == nullptr) {
            return ;
        }
        // 1. store a[i], N, NAC all a[...]
        if (index->type() != Ir::VAL_CONST) {
            // disable the array
            v.val.setValueNac(arr_target);
            return ;
        }
        // 2. store a[1], N
        //    first check whether all a[...] is disabled
        if (v.val.hasValue(arr_target) && v.val.isValueNac(arr_target)) {
            v.val.setValueNac(to_instr);
            v.val.clearArrayValue(arr_target);
            return ;
        }
        auto index_val = static_cast<Ir::Const*>(index)->v.imm_value().val.ival;
        //    then grep the value
        ValResult res = read_val(val, v);
        switch (res.stat) {
        case ValResult::VALUE:
#ifdef OPT_CONST_PROPAGATE_DEBUG
            printf("    SETVALUE %s[%llu] = %s \n", arr_target->name().c_str(), index_val, read_val(val, v).val.print().c_str());
#endif
            v.val.saveArrayValue(arr_target, index_val, read_val(val, v).val);
            break;
        case ValResult::NAC:
            v.val.setValueNac(to_instr);
            v.val.clearArrayValue(arr_target);
            break;
        case ValResult::UNDEF:
            break;
        }
    }

}

void when_load(Ir::LoadInstr* load, BlockValue& v)
{
    auto from = load->operand(0)->usee;
    fill_val(load, read_val(from, v), v);
}

void when_phi(Ir::PhiInstr *phi, BlockValue &v)
{
    for (auto [label, val] : *phi) {
        fill_val(phi, read_val(val, v), v);
        if (!v.val.hasValue(phi)) {
            v.val.setValueNac(phi);
            break;
        }
        if (v.val.isValueNac(phi)) {
            break;
        }
    }
}

void when_calculatable(Ir::CalculatableInstr* cal, BlockValue& v)
{
    Vector<ImmValue> vv;
    for (auto i : cal->operands()) {
        auto oprd = i->usee;
        ValResult res = read_val(oprd, v);
        switch (res.stat) {
        case ValResult::VALUE:
            vv.push_back(res.val);
            break;
        case ValResult::NAC:
            v.val.setValueNac(cal);
            return ;
        case ValResult::UNDEF:
            return ;
        }
    }
    v.val.setValue(cal, cal->calculate(vv));
}

void disable_minigep(Ir::Val* oprd, BlockValue &v)
{
    while (auto oprd_gep = dynamic_cast<Ir::MiniGepInstr*>(oprd)) {
        oprd = oprd_gep->operand(0)->usee;
    }
    if (auto alloc = dynamic_cast<Ir::AllocInstr*>(oprd)) {
        // printf("ALLOCA [%s] IS DISABLED\n", alloc->instr_print().c_str());
        v.val.setValueNac(alloc);
    }
}

void when_call(Ir::CallInstr* call, BlockValue &v)
{
    v.val.setValueNac(call); // all function regarded as NAK
    // when call builtin fill zero, erase front
    // it doesn't modify the mutability
    if (call->operand(0)->usee->name() == "__builtin_fill_zero") {
        if (call->operand(2)->usee->type() != Ir::VAL_CONST) {
            disable_minigep(call->operand(1)->usee, v);
            return ;
        }
        Ir::Instr* arr = dynamic_cast<Ir::AllocInstr*>(call->operand(1)->usee);

        if (arr == nullptr) {
            arr = dynamic_cast<Ir::CastInstr*>(call->operand(1)->usee);
            if (arr == nullptr) {
                return ;
            }
            arr = dynamic_cast<Ir::AllocInstr*>(arr->operand(0)->usee);
            if (arr == nullptr)
                return ;
        }

        int64_t len = static_cast<Ir::Const*>(call->operand(2)->usee)->v.imm_value().val.ival;
        v.val.eraseArrayValueLeq(arr, len);
        return ;
    }
    // all array arguments passed are NAK
    for (auto&& oprd_use : call->operands()) {
        disable_minigep(oprd_use->usee, v);
    }
}

void TransferFunction::operator()(Ir::Block *p, BlockValue &v) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
    printf("Analyzing Block %s\n", p->label()->name().c_str());
#endif
    for (const auto &i : *p) {
        if (auto instr = dynamic_cast<Ir::PhiInstr*>(i.get()); instr) {
            when_phi(instr, v);
        } else if (auto instr = dynamic_cast<Ir::StoreInstr*>(i.get()); instr) {
            when_store(instr, v);
        } else if (auto instr = dynamic_cast<Ir::LoadInstr*>(i.get()); instr) {
            when_load(instr, v);
        } else if (auto instr = dynamic_cast<Ir::CallInstr*>(i.get()); instr) {
            when_call(instr, v);
        } else if (auto instr = dynamic_cast<Ir::CalculatableInstr*>(i.get()); instr) {
            when_calculatable(instr, v);
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
        if (instr->instr_type() == Ir::INSTR_ALLOCA
#ifdef USING_MINI_GEP
            || instr->instr_type() == Ir::INSTR_MINI_GEP
#endif
            ) { return ; }
        if (instr->block() != p)
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
