#include "ir_cmp_instr.h"

namespace Ir {

String CmpInstr::instr_print() const {
    return name() + (is_float(operand(0)->usee->ty) ? " = fcmp " : " = icmp ") +
           gCmpInstrName[cmp_type] + " " + operand(0)->usee->ty->type_name() +
           " " + operand(0)->usee->name() + ", " + operand(1)->usee->name();
}

ImmValue CmpInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 2, "?");
    ImmValue &a1 = v[0];
    ImmValue &a2 = v[1];
    ImmValue ans;

    switch (cmp_type) {
    case CMP_EQ:
    case CMP_OEQ:
        ans = a1 == a2;
        break;
    case CMP_NE:
    case CMP_UNE:
        ans = a1 != a2;
        break;
    case CMP_OLT:
    case CMP_SLT:
    case CMP_ULT:
        ans = a1 < a2;
        break;
    case CMP_OGT:
    case CMP_SGT:
    case CMP_UGT:
        ans = a1 > a2;
        break;
    case CMP_OLE:
    case CMP_SLE:
    case CMP_ULE:
        ans = a1 <= a2;
        break;
    case CMP_OGE:
    case CMP_SGE:
    case CMP_UGE:
        ans = a1 >= a2;
        break;
    }
    // printf("[CmpInstr] %s\n", instr_print().c_str());
    // printf("    result = %s\n", ans.print().c_str());
    return ans;
}

pInstr make_cmp_instr(CmpType cmp_type, const pVal &a1, const pVal &a2) {
    return pInstr(new CmpInstr(cmp_type, a1, a2));
}

} // namespace Ir
