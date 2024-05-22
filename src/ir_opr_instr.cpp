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

    ImmValue ans;

    switch (binType) {
    case INSTR_ADD:
        ans = a0 + a1;
        break;
    case INSTR_SUB:
        ans = a0 - a1;
        break;
    case INSTR_MUL:
        ans = a0 * a1;
        break;
    case INSTR_DIV:
        ans = a0 / a1;
        break;
    case INSTR_SREM:
    case INSTR_UREM:
        ans = a0 % a1;
        break;
    case INSTR_AND:
        ans = a0 & a1;
        break;
    case INSTR_OR:
        ans = a0 | a1;
        break;
    case INSTR_XOR:
        ans = a0 ^ a1;
        break;
    }

    // printf("[BinaryInstr] %s\n", instr_print().c_str());
    // printf("    result = %s\n", ans.print().c_str());

    return ans;
}

ImmValue UnaryInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 1, "?");

    ImmValue ans = v[0].neg();

    // printf("[UnaryInstr] %s\n", instr_print().c_str());
    // printf("    result = %s\n", ans.print().c_str());

    return ans;
}

pInstr make_unary_instr(pVal oprd) { return pInstr(new UnaryInstr(oprd)); }

pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2) {
    return pInstr(new BinInstr(type, oprd1, oprd2));
}

} // namespace Ir
