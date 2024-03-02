#include "ir_instr.h"
#include "ir_block.h"
#include "ir_opr_instr.h"
#include "ir_call_instr.h"
#include "ir_constant.h"
#include "ir_func.h"

int main()
{
    Ir::Block block;
    Ir::pVal lastInstr;
    Ir::pFunc func = make_func(Ir::IMM_VOID, "hi", { Ir::IMM_I32, Ir::IMM_I32 } );
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_ADD, Ir::IMM_I32, Ir::make_constant(1), Ir::make_constant(2)));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_SUB, Ir::IMM_I32, Ir::make_constant(3), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_MUL, Ir::IMM_I32, Ir::make_constant(4), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_SDIV, Ir::IMM_I32, Ir::make_constant(5), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_REM, Ir::IMM_I32, Ir::make_constant(6), lastInstr));
    lastInstr = block.add_instr(Ir::make_call_instr(Ir::IMM_I32, func, { Ir::make_constant(6), lastInstr} ));
    lastInstr = block.add_instr(Ir::make_ret_instr(Ir::IMM_I32, lastInstr));
    printf("%s\n", block.print());
    return 0;
}
