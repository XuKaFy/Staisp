#include "ir_instr.h"
#include "ir_block.h"
#include "ir_constant.h"

int main()
{
    Ir::Block block;
    block.add_instr(Ir::make_binary_instr(Ir::INSTR_ADD, Ir::IMM_32, Ir::make_constant(1), Ir::make_constant(2)));
    block.add_instr(Ir::make_binary_instr(Ir::INSTR_SUB, Ir::IMM_32, Ir::make_constant(3), Ir::make_constant(4)));
    block.add_instr(Ir::make_binary_instr(Ir::INSTR_MUL, Ir::IMM_32, Ir::make_constant(5), Ir::make_constant(6)));
    block.add_instr(Ir::make_binary_instr(Ir::INSTR_SDIV, Ir::IMM_32, Ir::make_constant(7), Ir::make_constant(8)));
    block.add_instr(Ir::make_binary_instr(Ir::INSTR_REM, Ir::IMM_32, Ir::make_constant(9), Ir::make_constant(10)));
    printf("%s\n", block.print());
    return 0;
}
