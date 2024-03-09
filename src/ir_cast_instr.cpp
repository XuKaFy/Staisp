#include "ir_cast_instr.h"

namespace Ir
{

Symbol CastInstr::instr_print_impl() const
{
    auto tsrc = val->tr;
    auto tdest = tr;
    if(same_type(tsrc, tdest)) {
        my_assert(false, "how");
    }
    if(is_pointer(tsrc) && is_imm_type(tdest)) {
        return to_symbol(
            String(print_impl())
            + " = ptrtoint "
            + tsrc->type_name()
            + " "
            + val->print()
            + " to "
            + tr->type_name());
    }
    if(is_pointer(tdest) && is_imm_type(tsrc)) {
        return to_symbol(
            String(print_impl())
            + " = inttoptr "
            + tsrc->type_name()
            + " "
            + val->print()
            + " to "
            + tr->type_name());
    }
    if(bits_of_type(tdest) < bits_of_type(tsrc)) { // trunc
        return to_symbol(
            String(print_impl())
            + " = trunc "
            + tsrc->type_name()
            + " " 
            + val->print()
            + " to "
            + tdest->type_name());
    }
    if(bits_of_type(tdest) == bits_of_type(tsrc)) { // i32<->float, i64<->double
        if(is_float(tdest)) {
            if(is_signed_type(tsrc)) {
                return to_symbol(
                    String(print_impl())
                    + " = sitofp "
                    + tsrc->type_name()
                    + " " 
                    + val->print()
                    + " to "
                    + tdest->type_name());
            }
            return to_symbol(
                String(print_impl())
                + " = uitofp "
                + tsrc->type_name()
                + " " 
                + val->print()
                + " to "
                + tdest->type_name());
        }
        // tsrc must be float
        if(is_signed_type(tdest)) {
            return to_symbol(
                String(print_impl())
                + " = fptosi "
                + tsrc->type_name()
                + " " 
                + val->print()
                + " to "
                + tdest->type_name());
        }
        return to_symbol(
            String(print_impl())
            + " = fptoui "
            + tsrc->type_name()
            + " " 
            + val->print()
            + " to "
            + tdest->type_name());
    }
    // zext or sext
    if(is_signed_type(tdest)) {
        return to_symbol(
            String(print_impl())
            + " = sext "
            + tsrc->type_name()
            + " " 
            + val->print()
            + " to "
            + tdest->type_name());
    }
    return to_symbol(
        String(print_impl())
        + " = zext "
        + tsrc->type_name()
        + " " 
        + val->print()
        + " to "
        + tdest->type_name());
}

pInstr make_cast_instr(pType tr, pInstr a1)
{
    return pInstr(new CastInstr(tr, a1));
}

} // namespace ir
