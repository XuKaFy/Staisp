#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct RefInstr : public Instr {
    RefInstr(pInstr obj)
        :  Instr(INSTR_TYPE_NEED_REG, make_basic_type(ARCH_USED_POINTER_TYPE, false)), obj(obj) { }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

struct DerefInstr : public Instr {
    DerefInstr(pInstr obj, pType tr)
        :  Instr(INSTR_TYPE_NEED_REG, tr), obj(obj) {
        if(obj->tr->type_type() != TYPE_BASIC_TYPE || std::static_pointer_cast<BasicType>(obj->tr)->ty != ARCH_USED_POINTER_TYPE) {
            my_assert(false, "Pointer type must be IMM_U64");
        }
    }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

pInstr make_ref_instr(pInstr obj);
pInstr make_deref_instr(pInstr obj, pType tr);

} // namespace ir
