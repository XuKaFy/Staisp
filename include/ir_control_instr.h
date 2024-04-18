#pragma once

#include "ir_instr.h"

namespace Ir {

struct LabelInstr : public Instr
{
    LabelInstr()
        : Instr(make_label_type()) { }

    virtual Symbol instr_print_impl() const override;
};

struct BrInstr : public Instr {
    BrInstr(pInstr to)
        : Instr(make_void_type()) {
        add_operand(to);
    }

    virtual Symbol instr_print_impl() const override;
};

struct BrCondInstr : public Instr {
    BrCondInstr(pVal cond, pInstr trueTo, pInstr falseTo)
        : Instr(make_void_type()) {
        add_operand(cond);
        add_operand(trueTo);
        add_operand(falseTo);
    }

    virtual Symbol instr_print_impl() const override;
};

typedef Pointer<LabelInstr> pLabel;
typedef Pointer<BrInstr> pBrInstr;
typedef Pointer<BrCondInstr> pBrCondInstr;

pInstr make_label_instr();
pInstr make_br_instr(pInstr to);
pInstr make_br_cond_instr(pVal cond, pInstr trueTo, pInstr falseTo);

} // namespace ir
