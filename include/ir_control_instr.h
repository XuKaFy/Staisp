#pragma once

#include "ir_instr.h"

namespace Ir {

struct LabelInstr : public Instr {
    LabelInstr() : Instr(make_ir_type(IR_LABEL)) {}

    InstrType instr_type() const override { return INSTR_LABEL; }

    String instr_print() const override;
};

struct BrInstr : public Instr {
    BrInstr(const pInstr &to) : Instr(make_ir_type(IR_BR)) { add_operand(to); }

    BrInstr(Instr *to) : Instr(make_ir_type(IR_BR)) { add_operand(to); }

    InstrType instr_type() const override { return INSTR_BR; }

    String instr_print() const override;
};

struct BrCondInstr : public Instr {
    BrCondInstr(const pVal &cond, const pInstr &trueTo, const pInstr &falseTo)
        : Instr(make_ir_type(IR_BR_COND)) {
        add_operand(cond);
        add_operand(trueTo);
        add_operand(falseTo);
    }

    pInstr select(bool cond);

    InstrType instr_type() const override { return INSTR_BR_COND; }

    String instr_print() const override;
};

using pLabel = Pointer<LabelInstr>;
using pBrInstr = Pointer<BrInstr>;
using pBrCondInstr = Pointer<BrCondInstr>;

pInstr make_label_instr();
pInstr make_br_instr(const pInstr &to);
pInstr make_br_instr(Instr *to);
pInstr make_br_cond_instr(const pVal &cond, const pInstr &trueTo,
                          const pInstr &falseTo);

} // namespace Ir
