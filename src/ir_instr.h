#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

enum InstrType {
    INSTR_TYPE_NEED_REG,
    INSTR_TYPE_NO_REG,
    INSTR_TYPE_LABEL,
};

struct Instr : public Val {
    Instr(InstrType instrType, ImmType tr)
        : instrType(instrType), tr(tr) { }
    
    Symbol instr_print();
    virtual Symbol instr_print_impl() const;
    virtual Symbol print_impl() const override;

    virtual bool is_end_of_block() const = 0;

    InstrType instrType;
    ImmType tr;
    Symbol instr_str_form { nullptr };
    int line;
};

typedef Pointer<Instr> pInstr;

} // namespace Ir
