#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

enum InstrType {
    INSTR_TYPE_AS_ARG,
    INSTR_TYPE_BIN,
    INSTR_TYPE_UNARY,
    INSTR_TYPE_RET,
    INSTR_TYPE_CALL,
};

struct Instr : public Val {
    Instr(InstrType instrType, ImmType tr)
        : instrType(instrType), tr(tr) { }
    
    Symbol instr_print();
    virtual Symbol instr_print_impl() const;
    virtual Symbol print_impl() const override;

    InstrType instrType;
    ImmType tr;
    Symbol instr_str_form { nullptr };
    int line;
};

typedef Pointer<Instr> pInstr;

pInstr make_arg_instr(ImmType tr);

} // namespace Ir
