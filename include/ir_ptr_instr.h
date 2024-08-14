#pragma once

#include "def.h"
#include "ir_instr.h"
#include <iterator>

namespace Ir {

#ifdef USING_MINI_GEP
struct MiniGepInstr : public Instr {
    MiniGepInstr(Val* val, Val* index, bool in_this_dim = false);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_MINI_GEP; }

    Instr* clone_internal() const override {
        return new MiniGepInstr(operands()[0]->usee, operands()[1]->usee, in_this_dim);
    }

    // 寻址有两种
    // in_this_dim == true，直接在本层内寻址，表现为 GEP i32 N
    // in_this_dim == false，下一层寻址，表现为 GEP i32 0, i32 N
    const bool in_this_dim;
};

pInstr make_mini_gep_instr(Val* val, Val* index, bool in_this_dim = false);
#endif

struct ItemInstr : public Instr {
    ItemInstr(Val* val, const Vector<Val*> &index);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_ITEM; }

    Instr* clone_internal() const override {
        Vector<Val*> v;
        for (auto &&i = std::next(operands().begin()); i != operands().end(); ++i) {
            v.push_back((*i)->usee);
        }
        return new ItemInstr(operands()[0]->usee, v);
    }

    // from int[][10], false
    // from int[10][10], true
    bool get_from_local;
};

pInstr make_item_instr(Val* val, const Vector<Val*> &index);

} // namespace Ir
