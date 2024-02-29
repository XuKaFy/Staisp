#include "chunk.h"

void Chunk::push_mov(Reg left, Reg right)
{
    instructions.push_back(Instruction { INSTRUCTION_MOV64, { left, right }, {} });
}

void Chunk::push_mov(Reg left, Integer right)
{
    instructions.push_back(Instruction { INSTRUCTION_MOV64, { left, {} }, right });
}

void Chunk::push_opr(InstructionType ins, Reg l, Reg r)
{
    instructions.push_back(Instruction { ins, l, r });
}

void Chunk::print() const
{
    for(auto i : instructions) {
        switch(i.type) {
        case INSTRUCTION_MUL:
            printf("mul ");
            i.r[0].print();
            i.r[1].print();
            puts("");
            break;
        case INSTRUCTION_ADD:
            printf("add ");
            i.r[0].print();
            i.r[1].print();
            puts("");
            break;
        case INSTRUCTION_MOV32:
            printf("mov32 ");
            i.r[0].print();
            if(i.r[1].invaild()) {
                printf("%x", i.val);
            } else {
                i.r[1].print();
            }
            puts("");
            break;
        case INSTRUCTION_MOV64:
            printf("mov64 ");
            i.r[0].print();
            if(i.r[1].invaild()) {
                printf("%x", i.val);
            } else {
                i.r[1].print();
            }
            puts("");
            break;
        case INSTRUCTION_CALL:
            printf("call\n");
            break;
        case INSTRUCTION_RET:
            printf("ret\n");
            break;
        }
    }
}
