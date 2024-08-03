#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#ifdef USING_MINI_GEP
struct MiniGepInstr : public Instr {
    MiniGepInstr(const pVal &val, const pVal &index, bool in_this_dim = false);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_MINI_GEP; }

    // 寻址有两种
    // in_this_dim == true，直接在本层内寻址，表现为 GEP i32 N
    // in_this_dim == false，下一层寻址，表现为 GEP i32 0, i32 N
    const bool in_this_dim;
};

pInstr make_mini_gep_instr(const pVal &val, const pVal &index, bool in_this_dim = false);
#endif

struct ItemInstr : public Instr {
    ItemInstr(const pVal &val, const Vector<pVal> &index);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_ITEM; }

    // from int[][10], false
    // from int[10][10], true
    bool get_from_local;
};

pInstr make_item_instr(const pVal &val, const Vector<pVal> &index);

} // namespace Ir
