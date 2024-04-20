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
        return use("ptrtoint");
    }
    if(is_pointer(tdest) && is_basic_type(tsrc) && is_imm_integer(to_basic_type(tsrc)->ty)) {
        return use("inttoptr");
    }
    // 第二种转换：整数之间的降格
    if(is_integer(tdest) && is_integer(tsrc) && bytes_of_type(tdest) < bytes_of_type(tsrc)) { // trunc
        return use("trunc");
    }
    // 第三种转换：浮点数和整数之间的转换
    if(is_float(tdest) != is_float(tsrc)) {
        if(is_float(tdest)) {
            if(is_signed_type(tsrc)) {
                return use("sitofp");
            }
            return use("uitofp");
        }
        // tsrc must be float
        if(is_signed_type(tdest)) {
            return use("fptosi");
        }
        return use("fptoui");
    }
    // 第四种转换：整数之间的升格
    if(is_integer(tdest) && is_integer(tsrc) && bytes_of_type(tdest) > bytes_of_type(tsrc)) {
        // 注意，从 i1 进行升格必须使用 zext，否则会把 1 扩展为 -1
        if(is_signed_type(tdest) && to_basic_type(tsrc)->ty != IMM_I1) {
            return use("sext");
        }
        return use("zext");
    }
    // 第五种：浮点数直接的升降
    if(is_float(tdest) && is_float(tsrc)) {
        if(bytes_of_type(tdest) > bytes_of_type(tsrc)) 
            return use("fpext");
        return use("fptrunc");
    }
    printf("Converting type %s to type %s\n", tsrc->type_name(), tdest->type_name());
    throw Exception(2, "CastInstr", "not castable");
}

Symbol CastInstr::use(Symbol inst) const
{
    auto &val = operand(0)->usee;
    auto tsrc = val->ty;
    auto tdest = ty;
    return to_symbol(
        String(name())
        + " = "
        + inst
        + " "
        + tsrc->type_name()
        + " " 
        + val->name()
        + " to "
        + tdest->type_name());
}

ImmValue CastInstr::calculate(Vector<ImmValue> v) const
{
    my_assert(v.size() == 1, "?");
    ImmValue &from = v[0];

    return from.cast_to(to_basic_type(ty)->ty);
}

pInstr make_cast_instr(pType ty, pVal a1)
{
    return pInstr(new CastInstr(ty, a1));
}

} // namespace ir
