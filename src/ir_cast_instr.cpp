#include "ir_cast_instr.h"

namespace Ir
{

Symbol CastInstr::instr_print_impl() const
{
    auto tsrc = val->tr;
    auto tdest = tr;
    if(bits_of_type(tdest) < bits_of_type(tsrc)) { // trunc
        return to_symbol(
            String(print_impl())
            + " = trunc "
            + gImmName[tsrc]
            + " " 
            + val->print()
            + " to "
            + gImmName[tdest]);
    }
    if(bits_of_type(tdest) == bits_of_type(tsrc)) { // i32<->float, i64<->double
        if(is_float(tdest)) {
            if(is_signed_imm_type(tsrc)) {
                return to_symbol(
                    String(print_impl())
                    + " = sitofp "
                    + gImmName[tsrc]
                    + " " 
                    + val->print()
                    + " to "
                    + gImmName[tdest]);
            }
            return to_symbol(
                String(print_impl())
                + " = uitofp "
                + gImmName[tsrc]
                + " " 
                + val->print()
                + " to "
                + gImmName[tdest]);
        }
        // tsrc must be float
        if(is_signed_imm_type(tdest)) {
            return to_symbol(
                String(print_impl())
                + " = fptosi "
                + gImmName[tsrc]
                + " " 
                + val->print()
                + " to "
                + gImmName[tdest]);
        }
        return to_symbol(
            String(print_impl())
            + " = fptoui "
            + gImmName[tsrc]
            + " " 
            + val->print()
            + " to "
            + gImmName[tdest]);
    }
    // zext or sext
    if(is_signed_imm_type(tdest)) {
        return to_symbol(
            String(print_impl())
            + " = sext "
            + gImmName[tsrc]
            + " " 
            + val->print()
            + " to "
            + gImmName[tdest]);
    }
    return to_symbol(
        String(print_impl())
        + " = zext "
        + gImmName[tsrc]
        + " " 
        + val->print()
        + " to "
        + gImmName[tdest]);
}

pInstr make_cast_instr(ImmType tr, pInstr a1)
{
    return pInstr(new CastInstr(tr, a1));
}

} // namespace ir
