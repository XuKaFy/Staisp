#include "ir_opr_instr.h"
#include "imm.h"
#include "type.h"

namespace Ir {

String UnaryInstr::instr_print() const {
    return name() + " = " + "fneg" + " " + ty->type_name() + " " +
           operand(0)->usee->name();
}

String BinInstr::instr_print() const {
    return name() + " = " +
           (is_float(ty)
                ? "f"
                : (binType == INSTR_DIV ? (is_signed_type(ty) ? "s" : "u")
                                        : "")) +
           gBinInstrName[binType] + " " + ty->type_name() + " " +
           operand(0)->usee->name() + ", " + operand(1)->usee->name();
}

ImmValue BinInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 2, "?");
    ImmValue &a0 = v[0];
    ImmValue &a1 = v[1];

    switch (binType) {
    case INSTR_ADD:
        return a0 + a1;
    case INSTR_SUB:
        return a0 - a1;
    case INSTR_MUL:
        return a0 * a1;
    case INSTR_DIV:
        return a0 / a1;
    case INSTR_SREM:
    case INSTR_UREM:
        return a0 % a1;
    case INSTR_AND:
        return a0 & a1;
    case INSTR_OR:
        return a0 | a1;
    case INSTR_XOR:
        return a0 ^ a1;
    }
    throw Exception(1, "BinInstr::calculate", "?");
    return 0;
}

ImmValue UnaryInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 1, "?");
    return v[0].neg();
}

pInstr make_unary_instr(pVal oprd) { return pInstr(new UnaryInstr(oprd)); }

pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2) {
    return pInstr(new BinInstr(type, oprd1, oprd2));
}

} // namespace Ir
