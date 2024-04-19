#pragma once

#include "ir_instr.h"

namespace Ir {

#define CMP_INSTR_TABLE \
    ENTRY(EQ, eq) \
    ENTRY(NE, ne) \
    ENTRY(UGT, ugt) \
    ENTRY(UGE, uge) \
    ENTRY(ULT, ult) \
    ENTRY(ULE, ule) \
    ENTRY(SGT, sgt) \
    ENTRY(SGE, sge) \
    ENTRY(SLT, slt) \
    ENTRY(SLE, sle) \
    ENTRY(OEQ, oeq) \
    ENTRY(ONE, one) \
    ENTRY(OGT, ogt) \
    ENTRY(OGE, oge) \
    ENTRY(OLT, olt) \
    ENTRY(OLE, ole) \

#define ENTRY(x, y) CMP_##x,
enum CmpType {
    CMP_INSTR_TABLE
};
#undef ENTRY

#define ENTRY(x, y) #y,
const Symbol gCmpInstrName[] = {
    CMP_INSTR_TABLE
};
#undef ENTRY

#undef CMP_INSTR_TABLE

// 默认所有的操作数的类型都一致
struct CmpInstr : public Instr
{
    CmpInstr(CmpType cmp_type, pVal a1, pVal a2)
        : Instr(make_basic_type(IMM_I1)), cmp_type(cmp_type) {
        add_operand(a1);
        add_operand(a2);
    }
    
    virtual Symbol instr_print_impl() const override;
    CmpType cmp_type;
};

typedef Pointer<CmpInstr> pCmpInstr;

pInstr make_cmp_instr(CmpType cmp_type, pVal a1, pVal a2);

} // namespace ir
