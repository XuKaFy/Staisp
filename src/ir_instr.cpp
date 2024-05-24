#include "ir_instr.h"

#include <memory>

namespace Ir {

String Instr::instr_print() const { return name(); }

pInstr make_empty_instr() {
    static pInstr empty = pInstr();
    return empty;
}

pInstr make_sym_instr(const TypedSym &sym) {
    auto j = std::make_shared<Instr>(sym.ty);
    j->set_name(sym.sym);
    return j;
}

} // namespace Ir
