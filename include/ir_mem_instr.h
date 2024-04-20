#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct AllocInstr : public Instr {
    AllocInstr(pType ty)
        : Instr(make_pointer_type(ty)) { }
    
    virtual InstrType instr_type() const override {
        return INSTR_ALLOCA;
    }

    virtual Symbol instr_print_impl() const override;
};

struct LoadInstr : public Instr {
    LoadInstr(pVal from)
        : Instr(to_pointed_type(from->ty)) {
        add_operand(from);
    }

    virtual InstrType instr_type() const override {
        return INSTR_LOAD;
    }

    virtual Symbol instr_print_impl() const override;
};

struct StoreInstr : public Instr {
    StoreInstr(pVal to, pVal val)
        : Instr(make_ir_type(IR_STORE)) {
        add_operand(to);
        add_operand(val);
    }

    virtual InstrType instr_type() const override {
        return INSTR_STORE;
    }
    
    virtual Symbol instr_print_impl() const override;
};

pInstr make_alloc_instr(pType tr);
pInstr make_load_instr(pVal from);
pInstr make_store_instr(pVal to, pVal val);

} // namespace ir
