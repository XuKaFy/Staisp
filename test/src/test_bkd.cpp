#include "gtest/gtest.h"

#include "bkd_ir_instr.h"

using namespace Backend;

GTEST_TEST(bkd_ir, stringify) {
    ASSERT_EQ((MachineInstr {.instr =  RegRegInstr {
        .type = RegRegInstrType::ADD,
        .rd = Reg::T0,
        .rs1 = Reg::A1,
        .rs2 = Reg::A2,
    }}.stringify()), "add t0, a1, a2");

    ASSERT_EQ((MachineInstr {.instr =  CallInstr {
        .label = "printf",
    }}.stringify()), "call printf");
}