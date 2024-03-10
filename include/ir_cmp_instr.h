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
    ENTRY(SLE, sle) 

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

struct CmpInstr : public Instr
{
    CmpInstr(CmpType cmp_type, pType tr, pInstr a1, pInstr a2)
        : Instr(INSTR_TYPE_NEED_REG, make_basic_type(IMM_I1, false)), cmp_type(cmp_type), ch { a1, a2 } { }
    
    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }

    CmpType cmp_type;
    pInstr ch[2];
};

typedef Pointer<CmpInstr> pCmpInstr;

pInstr make_cmp_instr(CmpType cmp_type, pType tr, pInstr a1, pInstr a2);

} // namespace ir
