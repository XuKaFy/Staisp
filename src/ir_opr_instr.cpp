#include "ir_opr_instr.h"

#include "imm.h"

namespace Ir {

String UnaryInstr::instr_print() const {
    return name() + " = " + "fneg" + " " + ty->type_name() + " " +
           operand(0)->usee->name();
}

String BinInstr::instr_print() const {
    return name() + " = " + gBinInstrName[binType] + " " + ty->type_name() +
           " " + operand(0)->usee->name() + ", " + operand(1)->usee->name();
}

ImmValue BinInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 2, "?");
    ImmValue &a0 = v[0];
    ImmValue &a1 = v[1];

    ImmValue ans;

    switch (binType) {
    case INSTR_ADD:
    case INSTR_FADD:
        ans = a0 + a1;
        break;
    case INSTR_SUB:
    case INSTR_FSUB:
        ans = a0 - a1;
        break;
    case INSTR_MUL:
    case INSTR_FMUL:
        ans = a0 * a1;
        break;
    case INSTR_SDIV:
    case INSTR_UDIV:
    case INSTR_FDIV:
        ans = a0 / a1;
        break;
    case INSTR_SREM:
    case INSTR_UREM:
    case INSTR_FREM: // Yaossg's NOTE: XuKaFy did not implement floating point's
                     // remainder
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
    case INSTR_ASHR:
    case INSTR_LSHR: // Yaossg's NOTE: LSHR is not implemented
        ans = a0 >> a1;
    case INSTR_SHL:
        ans = a0 << a1;
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

pInstr make_unary_instr(const pVal &oprd) {
    return pInstr(new UnaryInstr(oprd));
}

pInstr make_binary_instr(BinInstrType type, const pVal &oprd1,
                         const pVal &oprd2) {
    return pInstr(new BinInstr(type, oprd1, oprd2));
}

} // namespace Ir
