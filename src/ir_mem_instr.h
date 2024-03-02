#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct AllocInstr : public Instr {
    AllocInstr(ImmType tr)
        : Instr(INSTR_TYPE_NEED_REG, tr) { }
    
    virtual Symbol instr_print_impl() const override;
};

struct LoadInstr : public Instr {
    LoadInstr(ImmType tr, pInstr from)
        : Instr(INSTR_TYPE_NEED_REG, tr), from(from) { }
    
    virtual Symbol instr_print_impl() const override;
    pInstr from;
};

struct StoreInstr : public Instr {
    StoreInstr(ImmType tr, pInstr to, pVal val)
        : Instr(INSTR_TYPE_NO_REG, tr), to(to), val(val) { }
    
    virtual Symbol instr_print_impl() const override;

    pInstr to;
    pVal val;
};

pInstr make_alloc_instr(ImmType tr);
pInstr make_load_instr(ImmType tr, pInstr from);
pInstr make_store_instr(ImmType tr, pInstr to, pVal val);

} // namespace ir
