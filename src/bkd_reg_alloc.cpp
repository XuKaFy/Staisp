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


bool BlockValue::operator==(const BlockValue &b) const {
    return uses == b.uses && fuses == b.fuses;
}

bool BlockValue::operator!=(const BlockValue &b) const {
    return !operator==(b);
}

void BlockValue::cup(const BlockValue &v) {
    for (const auto &i : v.uses) {
        uses.insert(i);
    }
    for (const auto &i : v.fuses) {
        fuses.insert(i);
    }
}

void BlockValue::clear() {
    uses.clear();
    fuses.clear();
}

void TransferFunction::operator()(const Block *p, BlockValue &v) {
    // in = use + (in - def)
    auto use = p->use();
    auto fuse = p->fuse();
    auto def = p->def();
    auto fdef = p->fdef();

    for (auto i : def) {
        v.uses.erase(i);
    }
    for (auto i : fdef) {
        v.fuses.erase(i);
    }
    for (auto i : use) {
        v.uses.insert(i);
    }
    for (auto i : fuse) {
        v.fuses.insert(i);
    }
}

int TransferFunction::operator()(const Block *p, const BlockValue &IN, const BlockValue &OUT) {

}

int Func::live_register_analysis() {
    Map<const Block *, BlockValue> INs;
    Map<const Block *, BlockValue> OUTs;
    std::deque<const Block *> pending_blocks;
    TransferFunction transfer;
    int ans = 0;
    for (const auto &i : body) {
        INs[&i] = BlockValue();
        OUTs[&i] = BlockValue();
        pending_blocks.push_back(&i);
    }
    while (!pending_blocks.empty()) {
        const Block *b = *pending_blocks.begin();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks();
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        transfer(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }
    for (const auto &i : body) {
        ans += transfer(&i, INs[&i], OUTs[&i]);
    }
    return ans;
}
}
