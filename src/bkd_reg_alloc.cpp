#include <bkd_reg_alloc.h>
#include <bkd_func.h>
#include <type.h>

namespace Backend {

void Func::allocate_register() {

}


Block Func::generate_prolog() const {
    Block bkd_block(name + "_prolog");
    auto add = [&bkd_block](MachineInstr const& value) {
        bkd_block.body.push_back(value);
    };
    {
        int sp = calculate_sp();
        add({ RegImmInstr {
            RegImmInstrType::ADDI, Reg::SP, Reg::SP, -sp
        } });
        add({ RegImmRegInstr {
            RegImmRegInstrType::SD, Reg::RA,  sp - 8, Reg::SP,
        } });
        add({ RegImmRegInstr {
            RegImmRegInstrType::SD, Reg::S0,  sp - 16, Reg::SP,
        } });
        add({ RegImmInstr {
            RegImmInstrType::ADDI, Reg::S0, Reg::SP, sp
        } });
    }
    Reg arg = Reg::A0;
    FReg farg = FReg::FA0;
    int sp = 0;
    int reg = 0;
    for (auto&& at : type->arg_type) {
        if (is_float(at)) {
            auto rd = (FReg) ~reg++;
            auto rs = farg;
            if (rs <= FReg::FA7) {
                add({  FRegInstr{
                    FRegInstrType::FMV_S, rd, rs
                } });
                farg = (FReg)((int)farg + 1);
            } else {
                add({ FRegImmRegInstr {
                    FRegImmRegInstrType::FLW, rd,  sp, Reg::SP,
                } });
                sp += 8;
            }
        } else {
            auto rd = (Reg) ~reg++;
            auto rs = arg;
            if (rs <= Reg::A7) {
                add({  RegInstr{
                    RegInstrType::MV, rd, rs
                } });
                arg = (Reg)((int)arg + 1);
            } else {
                add({ RegImmRegInstr {
                    RegImmRegInstrType::LD, rd,  sp, Reg::SP,
                } });
                sp += 8;
            }
        }
    }
    return bkd_block;
}

Block Func::generate_epilog() const {
    Block bkd_block(name + "_epilog");
    auto add = [&bkd_block](MachineInstr const& value) {
        bkd_block.body.push_back(value);
    };
    {
        int sp = calculate_sp();
        add({ RegImmRegInstr {
            RegImmRegInstrType::LD, Reg::RA,  8, Reg::SP,
        } });
        add({ RegImmRegInstr {
            RegImmRegInstrType::LD, Reg::S0,  0, Reg::SP,
        } });
        add({ RegImmInstr {
            RegImmInstrType::ADDI, Reg::SP, Reg::SP, sp
        } });
        add({ ReturnInstr{} });
    }
    return bkd_block;
}


}
