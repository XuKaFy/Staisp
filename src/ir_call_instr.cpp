#include "ir_call_instr.h"

namespace Ir {

Symbol CallInstr::instr_print_impl() const
{
    String res = String(print_impl())
        + " = call "
        + gImmName[tr]
        + " @"
        + func->print()
        + "("; // %1 = call @fun(i32 1, i64 2)
    
    my_assert(args.size() == func->args.size(), "Error: size of arguments unequal to the inputs");
    
    if(args.empty()) goto INSTR_PRINT_IMPL_END;

    res += gImmName[func->args[0].tr];
    res += " ";
    res += args[0]->print();
    
    for(size_t i=1; i<args.size(); ++i) {
        res += ", ";
        res += gImmName[func->args[i].tr];
        res += " ";
        res += args[i]->print();
    }

INSTR_PRINT_IMPL_END:
    res += ")";
    return to_symbol(res);
}

Symbol RetInstr::instr_print_impl() const
{
    return to_symbol(
        String("ret ")
        + gImmName[tr]
        + " "
        + ret->print());
}

pInstr make_call_instr(pFunc func, Vector<pInstr> args)
{
    return pInstr(new CallInstr(func, args));
}

pInstr make_ret_instr(ImmType tr, pInstr oprd)
{
    return pInstr(new RetInstr(tr, oprd));
}

} // namespace Ir
