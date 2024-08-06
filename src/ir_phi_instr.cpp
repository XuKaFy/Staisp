#include "ir_phi_instr.h"

namespace Ir {

PhiInstr::PhiInstr(const pType &type) : Instr(type) {};

void PhiInstr::remove(LabelInstr *label) {
    my_assert(operand_size() % 2 == 0, "?");
    // printf("PHI BEFORE REMOVE: %s, %lu\n", instr_print().c_str(),
    // operand_size());
    for (size_t i = 0; i < operand_size();) {
        if (!operand(i + 1)) {
            printf("EMPTY OPERAND\n");
            i += 2;
            continue;
        }
        LabelInstr *j = dynamic_cast<LabelInstr *>((operand(i + 1))->usee);
        if (j == label) {
            release_operand(i);
            release_operand(i);
        } else {
            ++(++i);
        }
    }
    // printf("PHI AFTER REMOVE: %s\n", instr_print().c_str());
    if (operand_size() == 2) {
        // printf("PHI INVAILD: %s\n", instr_print().c_str());
        replace_self(phi_val(0));
        block()->erase(this);
        // danger: do NOT execute anything after this line
    }
}

String PhiInstr::instr_print() const {
    /*
    Phi syntax:
    %indvar = phi i32 [ 0, %LoopHeader ], [ %nextindvar, %Loop ]
    <result> = phi [fast-math-flags] <ty> [ <val0>, <label0>],
    */
    my_assert(operand_size() % 2 == 0, "?");
    String ret;
    my_assert(ty->type_type() != TYPE_VOID_TYPE,
              "Phi Instruction type must be non-void");
    ret = name() + " = ";

    ret += "phi " + ty->type_name() + " ";
    for (auto [label, val] : *this) {
        ret += "[ ";
        ret += val->name();
        ret += ", %";
        ret += label->name();
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

std::shared_ptr<PhiInstr> make_phi_instr(const pType &type) {
    return std::make_shared<PhiInstr>(type);
}

}; // namespace Ir
