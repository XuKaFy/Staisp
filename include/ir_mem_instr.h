#pragma once

#include "def.h"
#include "ir_instr.h"
#include "ir_sym.h"

namespace Ir {

struct AllocInstr : public Instr {
    AllocInstr(pType tr)
        : Instr(INSTR_TYPE_ALLOC, make_pointer_type(tr, false)) { }
    
    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }
};

struct LoadInstr : public Instr {
    LoadInstr(pInstr from)
        : Instr(INSTR_TYPE_NEED_REG, to_pointed_type(from->tr)), from(from) { }
    
    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }
    pInstr from;
};

struct StoreInstr : public Instr {
    StoreInstr(pInstr to, pVal val)
        : Instr(INSTR_TYPE_NO_REG, to_pointed_type(to->tr)), to(to), val(val) { }
    
    virtual Symbol instr_print_impl() const override;
    virtual bool is_end_of_block() const { return false; }

    pInstr to;
    pVal val;
};

enum SymFrom {
    SYM_GLOBAL,
    SYM_LOCAL,
};

struct SymInstr : public Instr {
    SymInstr(TypedSym val, SymFrom from = SYM_LOCAL)
        : Instr(INSTR_TYPE_NO_REG, val.tr), sym(val.sym), from(from) { }

    virtual Symbol print_impl() const override;
    virtual bool is_end_of_block() const { return false; }
    Symbol sym;
    SymFrom from;
};

pInstr make_alloc_instr(pType tr);
pInstr make_load_instr(pInstr from);
pInstr make_store_instr(pInstr to, pVal val);
pInstr make_sym_instr(TypedSym val, SymFrom from = SYM_LOCAL);

} // namespace ir
