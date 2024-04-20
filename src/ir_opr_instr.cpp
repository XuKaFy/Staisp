#include "ir_opr_instr.h"

namespace Ir
{

Symbol UnaryInstr::instr_print_impl() const
{
    return to_symbol(
        String(name())
        + " = "
        + gUnaryInstrName[unaryType]
        + " " 
        + ty->type_name()
        + " "
        + operand(0)->usee->name());
}

Symbol BinInstr::instr_print_impl() const
{
    return to_symbol(
        String(name())
        + " = "
        + (is_float(ty) ? "f" : 
            (binType == INSTR_DIV ? (is_signed_type(ty) ? "s" : "u") : "")) 
        + gBinInstrName[binType] 
        + " " 
        + ty->type_name()
        + " "
        + operand(0)->usee->name()
        + ", "
        + operand(1)->usee->name());
}

ImmValue BinInstr::calculate(Vector<ImmValue> v) const
{
    my_assert(v.size() == 2, "?");
    ImmValue &a0 = v[0];
    ImmValue &a1 = v[1];

    switch(binType) {
    case INSTR_ADD:
        return a0 + a1;
    case INSTR_SUB:
        return a0 - a1;
    case INSTR_MUL:
        return a0 * a1;
    case INSTR_DIV:
        return a0 / a1;
    case INSTR_REM:
        return a0 % a1;
    case INSTR_AND:
        return a0 & a1;
    case INSTR_OR:
        return a0 | a1;
    }
    throw Exception(1, "BinInstr::calculate", "?");
    return 0;
}

ImmValue UnaryInstr::calculate(Vector<ImmValue> v) const
{
    my_assert(v.size() == 1, "?");
    ImmValue &a0 = v[0];
    
    switch(unaryType) {
    case INSTR_NOT:
        return !a0;
    case INSTR_NEG:
        return -a0;
    }
    throw Exception(1, "UnaryInstr::calculate", "?");
    return 0;
}

pInstr make_unary_instr(UnaryInstrType type, pVal oprd)
{
    return pInstr(new UnaryInstr(type, oprd));
}

pInstr make_binary_instr(BinInstrType type, pVal oprd1, pVal oprd2)
{
    return pInstr(new BinInstr(type, oprd1, oprd2));
}

} // namespace ir
