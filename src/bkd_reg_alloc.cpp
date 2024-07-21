#include <bkd_reg_alloc.h>
#include <bkd_func.h>
#include <type.h>

namespace Backend {

const std::vector<GReg> REG_ALLOC {
    Reg::A0,
    Reg::A1,
    Reg::A2,
    Reg::A3,
    Reg::A4,
    Reg::A5,
    Reg::A6,
    Reg::A7,

    FReg::FA0,
    FReg::FA1,
    FReg::FA2,
    FReg::FA3,
    FReg::FA4,
    FReg::FA5,
    FReg::FA6,
    FReg::FA7,

    // Reg::T0,
    // Reg::T1,
    // Reg::T2,
    Reg::T3,
    Reg::T4,
    Reg::T5,
    Reg::T6,


    // FReg::FT0,
    // FReg::FT1,
    // FReg::FT2,
    FReg::FT3,
    FReg::FT4,
    FReg::FT5,
    FReg::FT6,
    FReg::FT7,
    FReg::FT8,
    FReg::FT9,
    FReg::FT10,
    FReg::FT11,

    Reg::S0,
    Reg::S1,
    Reg::S2,
    Reg::S3,
    Reg::S4,
    Reg::S5,
    Reg::S6,
    Reg::S7,
    Reg::S8,
    Reg::S9,
    Reg::S10,
    Reg::S11,

    FReg::FS0,
    FReg::FS1,
    FReg::FS2,
    FReg::FS3,
    FReg::FS4,
    FReg::FS5,
    FReg::FS6,
    FReg::FS7,
    FReg::FS8,
    FReg::FS9,
    FReg::FS10,
    FReg::FS11,
};

void Func::allocate_hint() {
}

void Func::allocate_init() {
    for (auto&& [reg, ranges] : live_ranges) {
        if (is_virtual(reg)) {
            auto alloc_num = next_alloc_num();
            alloc_operand_map[alloc_num] = reg;
            alloc_range_map[alloc_num] = ranges;
            alloc_status_map[alloc_num] = AllocStatus::New;
        }
    }

    for (auto reg : REG_ALLOC) {
        occupied_map[reg] = Set<int>();
        occupied_range_map[reg] = std::set<AllocRange>();
    }
}

void Func::allocate_weight() {
}


Func::AllocPriority Func::get_alloc_priority(int alloc_num) {
    int block_cnt = 0;
    int instruction_cnt = 0;

    for (auto range : alloc_range_map[alloc_num]) {
        block_cnt++;
        instruction_cnt += range.instr_cnt;
    }

    auto operand_id = alloc_operand_map[alloc_num];
    bool hinted = coalesce_map.count(operand_id) || hint_map.count(operand_id);

    return std::make_tuple(alloc_status_map[alloc_num], block_cnt, instruction_cnt, hinted);
}


void Func::allocate_register() {

    for (auto &[alloc_num, _] : alloc_range_map) {
        alloc_priority_queue.emplace(get_alloc_priority(alloc_num), alloc_num);
    }

    while (!alloc_priority_queue.empty()) {
        auto priority_alloc = alloc_priority_queue.top();
        alloc_priority_queue.pop();

        auto alloc_num = priority_alloc.second;
        auto alloc_status = alloc_status_map[alloc_num];

        switch (alloc_status) {
            case AllocStatus::New:
            case AllocStatus::Assign:
                try_allocate(alloc_num);
            break;
            case AllocStatus::Split:
                try_split(alloc_num);
            break;
            case AllocStatus::Spill:
                spill(alloc_num);
            break;
            case AllocStatus::Memory:
            case AllocStatus::Done:
                break;
        }
    }
}

void Func::save_register() {

}

}
