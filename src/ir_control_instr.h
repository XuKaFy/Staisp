#pragma once

#include "ir_instr.h"

namespace Ir {

struct LabelInstr : public Instr
{
    LabelInstr()
        : Instr(INSTR_TYPE_LABEL, IMM_VOID) { }
    
    virtual Symbol instr_print_impl() const override;
};

struct BrInstr : public Instr {
    BrInstr(pInstr to)
        : Instr(INSTR_TYPE_NO_REG, IMM_VOID), to(to) {}

    virtual Symbol instr_print_impl() const override;

    pInstr to;
};

struct BrCondInstr : public Instr {
    BrCondInstr(pInstr cond, pInstr trueTo, pInstr falseTo)
        : Instr(INSTR_TYPE_NO_REG, cond->tr), cond(cond), trueTo(trueTo), falseTo(falseTo) {}

    virtual Symbol instr_print_impl() const override;

    pInstr cond;
    pInstr trueTo;
    pInstr falseTo;
};

typedef Pointer<LabelInstr> pLabel;
typedef Pointer<BrInstr> pBrInstr;
typedef Pointer<BrCondInstr> pBrCondInstr;

pInstr make_label_instr();
pInstr make_br_instr(pInstr to);
pInstr make_br_cond_instr(pInstr cond, pInstr trueTo, pInstr falseTo);

} // namespace ir
