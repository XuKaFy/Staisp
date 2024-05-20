#include "ir_instr.h"

namespace Ir {

String Instr::instr_print() const { return name(); }

pInstr make_empty_instr() {
    static pInstr empty = pInstr();
    return empty;
}

pInstr make_sym_instr(TypedSym sym) {
    auto j = pInstr(new Instr(sym.ty));
    j->set_name(sym.sym);
    return j;
}

} // namespace Ir
