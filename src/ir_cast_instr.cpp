#include "ir_cast_instr.h"

namespace Ir
{

Symbol CastInstr::instr_print_impl() const
{
    auto &val = operand(0)->usee;
    auto tsrc = val->ty;
    auto tdest = ty;
    if(is_same_type(tsrc, tdest)) {
        throw Exception(1, "CastInstr", "no need to cast");
    }
    // 第一种转换：在整数和指针之间转换
    if(is_pointer(tsrc) && is_basic_type(tdest) && is_imm_integer(to_basic_type(tdest)->ty)) {
        return to_symbol(
            String(name())
            + " = ptrtoint "
            + tsrc->type_name()
            + " "
            + val->name()
            + " to "
            + ty->type_name());
    }
    if(is_pointer(tdest) && is_basic_type(tsrc) && is_imm_integer(to_basic_type(tsrc)->ty)) {
        return to_symbol(
            String(name())
            + " = inttoptr "
            + tsrc->type_name()
            + " "
            + val->name()
            + " to "
            + ty->type_name());
    }
    // 第二种转换：整数之间的降格
    if(is_integer(tdest) && is_integer(tsrc) && bytes_of_type(tdest) < bytes_of_type(tsrc)) { // trunc
        return to_symbol(
            String(name())
            + " = trunc "
            + tsrc->type_name()
            + " " 
            + val->name()
            + " to "
            + tdest->type_name());
    }
    // 第三种转换：浮点数和整数之间的转换
    if(is_float(tdest) != is_float(tsrc)) {
        if(is_float(tdest)) {
            if(is_signed_type(tsrc)) {
                return to_symbol(
                    String(name())
                    + " = sitofp "
                    + tsrc->type_name()
                    + " " 
                    + val->name()
                    + " to "
                    + tdest->type_name());
            }
            return to_symbol(
                String(name())
                + " = uitofp "
                + tsrc->type_name()
                + " " 
                + val->name()
                + " to "
                + tdest->type_name());
        }
        // tsrc must be float
        if(is_signed_type(tdest)) {
            return to_symbol(
                String(name())
                + " = fptosi "
                + tsrc->type_name()
                + " " 
                + val->name()
                + " to "
                + tdest->type_name());
        }
        return to_symbol(
            String(name())
            + " = fptoui "
            + tsrc->type_name()
            + " " 
            + val->name()
            + " to "
            + tdest->type_name());
    }
    // 第四种转换：整数之间的升格
    if(is_integer(tdest) && is_integer(tsrc) && bytes_of_type(tdest) > bytes_of_type(tsrc)) {
        // 注意，从 i1 进行升格必须使用 zext，否则会把 1 扩展为 -1
        if(is_signed_type(tdest) && to_basic_type(tsrc)->ty != IMM_I1) {
            return to_symbol(
                String(name())
                + " = sext "
                + tsrc->type_name()
                + " " 
                + val->name()
                + " to "
                + tdest->type_name());
        }
        return to_symbol(
            String(name())
            + " = zext "
            + tsrc->type_name()
            + " " 
            + val->name()
            + " to "
            + tdest->type_name());
    }
    throw Exception(2, "CastInstr", "not castable");
}

pInstr make_cast_instr(pType ty, pVal a1)
{
    return pInstr(new CastInstr(ty, a1));
}

} // namespace ir
