#include "evaluate.h"

static Reg opt_return(Chunk &c, Pointer<Node> n, Regs &regs)
{
    Reg temp;
    // n cannot be type of { assign }
    if(is_immediate(n)) {
        temp = evaluate(c, n, regs);
    } else {
        Reg retReg = evaluate(c, n, regs);
        temp = regs.allocate();
        c.push_mov(temp, retReg);
        regs.free(retReg);
    }
    return temp;
}

Reg evaluate(Chunk &c, Pointer<Node> n, Regs &regs)
{
    if(is_immediate(n)) {
        Reg tempReg = regs.allocate();
        c.push_mov(tempReg, n->val);
        return tempReg;
    }

    if(is_assign(n)) {
        Reg var = regs.var(left_of(n)->name);
        Reg temp = opt_return(c, right_of(n), regs);
        c.push_mov(var, temp);
        return Reg::null();
    }

    Reg tempLeftReg = opt_return(c, left_of(n), regs);
    Reg tempRightReg = opt_return(c, right_of(n), regs);

    c.push_opr(convertTypeFromNodeToInstruction(OperationType(n->val)), tempLeftReg, tempRightReg);
    regs.free(tempRightReg);
    return tempLeftReg;
}

InstructionType convertTypeFromNodeToInstruction(OperationType t)
{
    switch(t) {
    case NODE_ADD:
        return INSTRUCTION_ADD;
    case NODE_MUL:
        return INSTRUCTION_MUL;
    }
    return INSTRUCTION_UNKNOWN;
}
