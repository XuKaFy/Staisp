#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

#ifdef USING_MINI_GEP
pType ex_shell_1(pType ty);

struct MiniGepInstr : public Instr {
    MiniGepInstr(const pVal &val, const pVal &index);

    String instr_print() const override;

    InstrType instr_type() const override { return INSTR_MINI_GEP; }
};

pInstr make_mini_gep_instr(const pVal &val, const pVal &index);
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
