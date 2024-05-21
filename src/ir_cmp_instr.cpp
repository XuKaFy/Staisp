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

    switch (cmp_type) {
    case CMP_EQ:
    case CMP_OEQ:
        return a1 == a2;
    case CMP_NE:
    case CMP_ONE:
    case CMP_UNE:
        return a1 != a2;
    case CMP_OLT:
    case CMP_SLT:
    case CMP_ULT:
        return a1 < a2;
    case CMP_OGT:
    case CMP_SGT:
    case CMP_UGT:
        return a1 > a2;
    case CMP_OLE:
    case CMP_SLE:
    case CMP_ULE:
        return a1 <= a2;
    case CMP_OGE:
    case CMP_SGE:
    case CMP_UGE:
        return a1 >= a2;
    }
    throw Exception(1, "CmpInstr::calculate", "?");
    return 0;
}

pInstr make_cmp_instr(CmpType cmp_type, pVal a1, pVal a2) {
    return pInstr(new CmpInstr(cmp_type, a1, a2));
}

} // namespace Ir
