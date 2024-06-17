#include "ir_phi_instr.h"

namespace Ir {

PhiInstr::PhiInstr(const pType &type)
    : Instr(is_pointer(type) ? to_pointed_type(type) : type) {};

String PhiInstr::instr_print() const {
    /*
    Phi syntax:
    %indvar = phi i32 [ 0, %LoopHeader ], [ %nextindvar, %Loop ]
    <result> = phi [fast-math-flags] <ty> [ <val0>, <label0>],
    */
    String ret;
    my_assert(ty->type_type() != TYPE_VOID_TYPE,
              "Phi Instruction type must be non-void");
    ret = name() + " = ";

    ret += "phi " + ty->type_name() + " ";
    for (size_t index = 0; index < operands.size() / 2; ++index) {
        auto val = phi_val(index);
        auto lab_use = phi_label(index);
        ret += "[ ";
        ret += val->name();
        ret += ", %";
        ret += lab_use->name();
        ret += " ], ";
    }

    ret.pop_back();
    ret.pop_back();
    return ret;
}

void PhiInstr::add_incoming(LabelInstr *blk, Val *val) {
    my_assert(is_same_type(val->ty, ty), "operand is same as type of phi node");
    add_operand(val);
    add_operand(blk);
}

pInstr make_phi_instr(const pType &type) {
    return std::make_shared<PhiInstr>(type);
}

}; // namespace Ir
