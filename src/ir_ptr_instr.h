#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct RefInstr : public Instr {
    RefInstr(pInstr obj)
        :  Instr(INSTR_TYPE_NEED_REG, IMM_U64), obj(obj) { }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

struct DerefInstr : public Instr {
    DerefInstr(pInstr obj, ImmType tr)
        :  Instr(INSTR_TYPE_NEED_REG, tr), obj(obj) {
        my_assert(obj->tr == IMM_U64, "Pointer type must be IMM_U64");
    }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

pInstr make_ref_instr(pInstr obj);
pInstr make_deref_instr(pInstr obj, ImmType tr);

} // namespace ir
