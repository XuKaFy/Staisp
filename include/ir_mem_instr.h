#pragma once

#include <utility>

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct AllocInstr : public Instr {
    AllocInstr(pType ty) : Instr(make_pointer_type(std::move(ty))) {}

    InstrType instr_type() const override { return INSTR_ALLOCA; }

    String instr_print() const override;
};

struct LoadInstr : public Instr {
    LoadInstr(const pVal &from) : Instr(to_pointed_type(from->ty)) {
        add_operand(from);
    }

    InstrType instr_type() const override { return INSTR_LOAD; }

    String instr_print() const override;
};

struct StoreInstr : public Instr {
    StoreInstr(const pVal &to, const pVal &val)
        : Instr(make_ir_type(IR_STORE)) {
        add_operand(to);
        add_operand(val);
    }

    InstrType instr_type() const override { return INSTR_STORE; }

    String instr_print() const override;
};

pInstr make_alloc_instr(pType tr);
pInstr make_load_instr(const pVal &from);
pInstr make_store_instr(const pVal &to, const pVal &val);

} // namespace Ir
