#pragma once

#include <utility>

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct AllocInstr : public Instr {
    AllocInstr(pType ty) : Instr(make_pointer_type(std::move(ty))) {}

    InstrType instr_type() const override { return INSTR_ALLOCA; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new AllocInstr(ty);
    }

    String instr_print() const override;
};

struct LoadInstr : public Instr {
    LoadInstr(Val* from) : Instr(to_pointed_type(from->ty)) {
        add_operand(from);
    }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new LoadInstr(new_operands.front());
    }

    InstrType instr_type() const override { return INSTR_LOAD; }

    String instr_print() const override;
};

struct StoreInstr : public Instr {
    StoreInstr(Val *to, Val *val)
        : Instr(make_ir_type(IR_STORE)) {
        add_operand(to);
        add_operand(val);
    }

    InstrType instr_type() const override { return INSTR_STORE; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new StoreInstr(new_operands.front(), new_operands.back());
    }

    String instr_print() const override;
};

pInstr make_alloc_instr(pType tr);
pInstr make_load_instr(Val* from);
pInstr make_store_instr(Val *to, Val *val);

} // namespace Ir
