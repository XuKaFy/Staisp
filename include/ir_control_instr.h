#pragma once

#include "ir_instr.h"

namespace Ir {

struct LabelInstr : public Instr {
    LabelInstr() : Instr(make_ir_type(IR_LABEL)) {}

    InstrType instr_type() const override { return INSTR_LABEL; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new LabelInstr();
    }

    String instr_print() const override;
};

struct BrInstr : public Instr {
    BrInstr(Instr *to) : Instr(make_ir_type(IR_BR)) { add_operand(to); }

    InstrType instr_type() const override { return INSTR_BR; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new BrInstr(dynamic_cast<Instr*>(new_operands.front()));
    }

    String instr_print() const override;
};

struct BrCondInstr : public Instr {
    BrCondInstr(Val* cond, Instr* trueTo, Instr* falseTo)
        : Instr(make_ir_type(IR_BR_COND)) {
        add_operand(cond);
        add_operand(trueTo);
        add_operand(falseTo);
    }

    pInstr select(bool cond);

    InstrType instr_type() const override { return INSTR_BR_COND; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new BrCondInstr(new_operands[0],
                               dynamic_cast<Instr*>(new_operands[1]),
                               dynamic_cast<Instr*>(new_operands[2]));
    }

    String instr_print() const override;
};

using pLabel = Pointer<LabelInstr>;
using pBrInstr = Pointer<BrInstr>;
using pBrCondInstr = Pointer<BrCondInstr>;

pInstr make_label_instr();
pInstr make_br_instr(Instr *to);
pInstr make_br_cond_instr(Val* cond, Instr* trueTo, Instr* falseTo);

} // namespace Ir
