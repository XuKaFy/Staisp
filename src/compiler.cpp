#include "ir_instr.h"
#include "ir_block.h"
#include "ir_constant.h"

int main()
{
    Ir::Block block;
    Ir::pVal lastInstr;
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_ADD, Ir::IMM_32, Ir::make_constant(1), Ir::make_constant(2)));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_SUB, Ir::IMM_32, Ir::make_constant(3), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_MUL, Ir::IMM_32, Ir::make_constant(4), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_SDIV, Ir::IMM_32, Ir::make_constant(5), lastInstr));
    lastInstr = block.add_instr(Ir::make_binary_instr(Ir::INSTR_REM, Ir::IMM_32, Ir::make_constant(6), lastInstr));
    printf("%s\n", block.print());
    return 0;
}
