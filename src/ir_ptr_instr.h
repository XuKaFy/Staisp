#pragma once

#include "def.h"
#include "ir_instr.h"

namespace Ir {

struct RefInstr : public Instr {
    RefInstr(pInstr obj)
        :  Instr(INSTR_TYPE_NEED_REG, make_pointer_type(obj->tr, false)), obj(obj) { }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

struct DerefInstr : public Instr {
    DerefInstr(pInstr obj)
        :  Instr(INSTR_TYPE_NEED_REG, to_pointer(obj->tr)->pointed_type), obj(obj) { }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr obj;
};

struct ItemInstr : public Instr {
    ItemInstr(pInstr val, pInstr index)
        :  Instr(INSTR_TYPE_NEED_REG, make_pointer_type(to_elem_type(to_pointed_type(val->tr)), false)), val(val), index(index) { }

    virtual bool is_end_of_block() const { return false; }
    virtual Symbol instr_print_impl() const override;

    pInstr val;
    pInstr index;
};

pInstr make_ref_instr(pInstr obj);
pInstr make_deref_instr(pInstr obj);
pInstr make_item_instr(pInstr val, pInstr index);

} // namespace ir
