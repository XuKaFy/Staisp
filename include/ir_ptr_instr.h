#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#ifdef USING_MINI_GEP
struct MiniGepInstr : public Instr {
    MiniGepInstr(Val* val, Val* index, bool in_this_dim = false);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_MINI_GEP; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new MiniGepInstr(new_operands[0], new_operands[1], in_this_dim);
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

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new ItemInstr(new_operands[0], 
                Vector<Val*>(std::next(new_operands.begin()), new_operands.end()));
    }

    // from int[][10], false
    // from int[10][10], true
    bool get_from_local;
};

pInstr make_item_instr(Val* val, const Vector<Val*> &index);

} // namespace Ir
