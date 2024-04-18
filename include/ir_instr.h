#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "ir_val.h"

namespace Ir {

struct Instr : public User {
    Instr(pType ty)
        : User(ty) { }

    Symbol instr_print();
    virtual Symbol instr_print_impl() const;

private:
    Symbol instr_str_form { nullptr };
};

typedef Pointer<Instr> pInstr;
typedef Vector<pInstr> Instrs;

pInstr make_empty_instr();
pInstr make_sym_instr(TypedSym sym);

} // namespace Ir
