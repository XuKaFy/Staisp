#include "ir_instr.h"
#include "ir_block.h"
#include "ir_opr_instr.h"
#include "ir_call_instr.h"
#include "ir_constant.h"
#include "ir_func.h"
#include "ir_func_defined.h"
#include "ir_module.h"

Ir::pFuncDefined a_plus_b(Ir::pGlobal g)
{
    Ir::pFuncDefined func = make_func_defined(Ir::IMM_I32, Ir::make_sym("hi"), { Ir::IMM_I32, Ir::IMM_I32 });

    Ir::pBlock first_block = func->body.front();
    Ir::pBlock block = Ir::make_block();
    Ir::pVal lastInstr;

    lastInstr = block->add_instr(Ir::make_binary_instr(Ir::INSTR_ADD, Ir::IMM_I32, first_block->instrs[0], first_block->instrs[1]));
    lastInstr = block->add_instr(Ir::make_binary_instr(Ir::INSTR_MUL, Ir::IMM_I32, lastInstr, g));
    lastInstr = block->add_instr(Ir::make_ret_instr(Ir::IMM_I32, lastInstr));

    func->add_block(block);

    return func;
}

int main()
{
    Ir::Module mod;
    Ir::pGlobal g = Ir::make_global(Ir::IMM_I32, 30, Ir::make_sym("a"));
    mod.add_global(g);
    mod.add_func(a_plus_b(g));
    printf("%s\n", mod.print_module());
    return 0;
}
