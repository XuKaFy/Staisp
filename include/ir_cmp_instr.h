#pragma once

#include "ir_instr.h"

namespace Ir {

#define CMP_INSTR_TABLE                                                        \
    ENTRY(EQ, eq)                                                              \
    ENTRY(NE, ne)                                                              \
                                                                               \
    ENTRY(UGT, ugt)                                                            \
    ENTRY(UGE, uge)                                                            \
    ENTRY(ULT, ult)                                                            \
    ENTRY(ULE, ule)                                                            \
                                                                               \
    ENTRY(SGT, sgt)                                                            \
    ENTRY(SGE, sge)                                                            \
    ENTRY(SLT, slt)                                                            \
    ENTRY(SLE, sle)                                                            \
                                                                               \
    ENTRY(OEQ, oeq)                                                            \
    ENTRY(OGT, ogt)                                                            \
    ENTRY(OGE, oge)                                                            \
    ENTRY(OLT, olt)                                                            \
    ENTRY(OLE, ole)                                                            \
    ENTRY(UNE, une)

#define ENTRY(x, y) CMP_##x,
enum CmpType { CMP_INSTR_TABLE };
#undef ENTRY

#define ENTRY(x, y) #y,
const String gCmpInstrName[] = {CMP_INSTR_TABLE};
#undef ENTRY

#undef CMP_INSTR_TABLE

// 默认所有的操作数的类型都一致
struct CmpInstr : public CalculatableInstr {
    CmpInstr(CmpType cmp_type, const pVal &a1, const pVal &a2)
        : CalculatableInstr(make_basic_type(IMM_I1)), cmp_type(cmp_type) {
        add_operand(a1);
        add_operand(a2);
    }

    InstrType instr_type() const override { return INSTR_CMP; }

    String instr_print() const override;

    ImmValue calculate(Vector<ImmValue> v) const override;

    CmpType cmp_type;
};

using pCmpInstr = Pointer<CmpInstr>;

pInstr make_cmp_instr(CmpType cmp_type, const pVal &a1, const pVal &a2);

} // namespace Ir
