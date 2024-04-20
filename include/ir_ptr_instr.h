#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct ItemInstr : public Instr {
    ItemInstr(pVal val, Vector<pVal> index);

    virtual Symbol instr_print_impl() const override;

    virtual InstrType instr_type() const override {
        return INSTR_ITEM;
    }

    Vector<pVal> index;
};

pInstr make_item_instr(pVal val, Vector<pVal> index);

} // namespace ir
