#include "ir_cast_instr.h"

namespace Ir
{

Symbol CastInstr::instr_print_impl() const
{
    auto tsrc = val->tr;
    auto tdest = tr;
    String ans = print_impl();
    // TODO TODO TODO
    return to_symbol(ans);
}

pInstr make_cast_instr(ImmType tr, pInstr a1)
{
    return pInstr(new CastInstr(tr, a1));
}

} // namespace ir
