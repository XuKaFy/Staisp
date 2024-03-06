#pragma once

#include "def.h"

#include "ir_val.h"

namespace Ir {

enum InstrType {
    INSTR_TYPE_NEED_REG,
    INSTR_TYPE_NO_REG,
    INSTR_TYPE_LABEL,
    INSTR_TYPE_HIDE,
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

struct ConstArgInstr : public Instr {
    ConstArgInstr(TypedSym sym);
    virtual Symbol instr_print_impl() const override;
    virtual Symbol print_impl() const override;
    virtual bool is_end_of_block() const override;

    Symbol sym;
};

pInstr make_empty_instr();
pInstr make_const_arg(TypedSym sym);

} // namespace Ir
