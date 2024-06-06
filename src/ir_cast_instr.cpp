#include "ir_cast_instr.h"

#include "imm.h"
#include "type.h"

namespace Ir {

String CastInstr::instr_print() const {
    auto &val = operand(0)->usee;
    auto tsrc = val->ty;
    auto tdest = ty;
    return name() + " = " + CAST_METHOD_NAME[_method] + " " + tsrc->type_name() + " " + val->name() +
           " to " + tdest->type_name();
}

CastMethod get_cast_method(pType tsrc, pType tdest)
{
    if (is_same_type(tsrc, tdest)) {
        throw Exception(1, "CastInstr", "no need to cast");
    }
    // 第零种转换：指针互转
    if (is_pointer(tsrc) && is_pointer(tdest)) {
        return CAST_BITCAST;
    }
    // 第一种转换：在整数和指针之间转换
    if (is_pointer(tsrc) && is_basic_type(tdest) &&
        is_imm_integer(to_basic_type(tdest)->ty)) {
        return CAST_PTRTOINT;
    }
    if (is_pointer(tdest) && is_basic_type(tsrc) &&
        is_imm_integer(to_basic_type(tsrc)->ty)) {
        return CAST_INTTOPTR;
    }
    // 第二种转换：整数之间的降格
    if (is_integer(tdest) && is_integer(tsrc) &&
        bytes_of_type(tdest) < bytes_of_type(tsrc)) { // trunc
        return CAST_TRUNC;
    }
    // 第三种转换：浮点数和整数之间的转换
    if (is_float(tdest) != is_float(tsrc)) {
        if (is_float(tdest)) {
            if (is_signed_type(tsrc)) {
                return CAST_SITOFP;
            }
            return CAST_UITOFP;
        }
        // tsrc must be float
        if (is_signed_type(tdest)) {
            return CAST_FPTOSI;
        }
        return CAST_FPTOUI;
    }
    // 第四种转换：整数之间的升格
    if (is_integer(tdest) && is_integer(tsrc) &&
        bytes_of_type(tdest) > bytes_of_type(tsrc)) {
        // 注意，从 i1 进行升格必须使用 zext，否则会把 1 扩展为 -1
        if (is_signed_type(tdest) && to_basic_type(tsrc)->ty != IMM_I1) {
            return CAST_SEXT;
        }
        return CAST_ZEXT;
    }
    // 第五种：浮点数直接的升降
    if (is_float(tdest) && is_float(tsrc)) {
        if (bytes_of_type(tdest) > bytes_of_type(tsrc)) {
            return CAST_FPEXT;
        }
        return CAST_FPTRUNC;
    }
    printf("Warning: Converting type %s to type %s\n",
           tsrc->type_name().c_str(), tdest->type_name().c_str());
    throw Exception(2, "CastInstr", "not castable");
    return CAST_BITCAST;
}

ImmValue CastInstr::calculate(Vector<ImmValue> v) const {
    my_assert(v.size() == 1, "?");
    ImmValue &from = v[0];

    ImmValue ans = from.cast_to(to_basic_type(ty)->ty);
    // printf("[CastInstr] %s\n", instr_print().c_str());
    // printf("    result = %s\n", ans.print().c_str());

    return ans;
}

pInstr make_cast_instr(pType ty, const pVal &a1) {
    return pInstr(new CastInstr(std::move(ty), a1));
}

} // namespace Ir
