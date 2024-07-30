#include <bkd_reg_alloc.h>
#include <bkd_func.h>

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
    for (auto&& block : blocks) {
        for (auto&& instr : block.body) {
            if (instr.instr_type() == MachineInstr::Type::REG) {
                auto [type, rd, rs] = instr.as<RegInstr>();
                if (type == RegInstrType::MV) {
                    if (is_virtual(rd) && is_virtual(rs)) {
                        coalesce_map[rd] = rs;
                        coalesce_map[rs] = rd;
                    } else if (is_virtual(rd)) {
                        hint_map[rd] = rs;
                    } else if (is_virtual(rs)) {
                        hint_map[rs] = rd;
                    }
                }
            }
            if (instr.instr_type() == MachineInstr::Type::FREG) {
                auto [type, rd, rs] = instr.as<FRegInstr>();
                if (type == FRegInstrType::FMV_S) {
                    if (is_virtual(rd) && is_virtual(rs)) {
                        coalesce_map[rd] = rs;
                        coalesce_map[rs] = rd;
                    } else if (is_virtual(rd)) {
                        hint_map[rd] = rs;
                    } else if (is_virtual(rs)) {
                        hint_map[rs] = rd;
                    }
                }
            }
        }
    }
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
    for (auto&& block : blocks) {
        block_weight_map[&block] = 1.0;
    }
    // DOM tree analysis needed here
    // omitted for simplicity
}


double Func::get_range_weight(const LiveRange& range) {
    return range.instr_cnt * block_weight_map[range.block];
}

double Func::get_spill_weight(int alloc_num) {
    double weight = 0;
    for (auto range : alloc_range_map[alloc_num]) {
        weight += get_range_weight(range);
    }

    auto operand_id = alloc_operand_map[alloc_num];
    if (coalesce_map.count(operand_id)) {
        weight *= 1.5;
    } else if (hint_map.count(operand_id)) {
        weight *= 2.0;
    }
    return weight;
}

Func::PrioritizedAlloc Func::get_alloc_priority(int alloc_num) {
    int block_cnt = 0;
    int instruction_cnt = 0;

    for (auto range : alloc_range_map[alloc_num]) {
        block_cnt++;
        instruction_cnt += range.instr_cnt;
    }

    auto operand_id = alloc_operand_map[alloc_num];
    bool hinted = coalesce_map.count(operand_id) || hint_map.count(operand_id);

    return {alloc_num, std::make_tuple(alloc_status_map[alloc_num], block_cnt, instruction_cnt, hinted)};
}


void Func::allocate_register() {

    for (auto &[alloc_num, _] : alloc_range_map) {
        alloc_priority_queue.emplace(get_alloc_priority(alloc_num));
    }

    while (!alloc_priority_queue.empty()) {
        auto priority_alloc = alloc_priority_queue.top();
        alloc_priority_queue.pop();

        auto alloc_num = priority_alloc.alloc_num;
        auto alloc_status = std::get<0>(priority_alloc.priority);

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

void Func::try_allocate(int alloc_num) {
    auto operand = alloc_operand_map[alloc_num];
    const auto& range_list = alloc_range_map[alloc_num];

    std::optional<GReg> allocated_reg = std::nullopt;

    Map<GReg, std::set<int>> conflict_map;
    std::set<GReg> conflict_hard_set;
    std::vector<GReg> register_alloc_vector;
    register_alloc_vector.reserve(REG_ALLOC.size());

    if (hint_map.count(operand)) {
        if (std::find(REG_ALLOC.begin(), REG_ALLOC.end(), hint_map[operand]) != REG_ALLOC.end()) {
            register_alloc_vector.push_back(hint_map[operand]);
        }
        for (auto&& reg : REG_ALLOC) {
            if (!(reg == hint_map[operand])) {
                register_alloc_vector.push_back(reg);
            }
        }
    } else {
        register_alloc_vector = REG_ALLOC;
    }

    for (auto&& reg : register_alloc_vector) {
        // must be both int or both float
        if (reg.index() != operand.index())
            continue;

        // liveness: sorted, begin with smallest.
        auto alloc_range = AllocRange(range_list[0], alloc_num);
        bool is_conflict = false;
        conflict_map[reg] = std::set<int>();

        for (auto &reg_range : live_ranges[reg]) {
            for (auto &range : range_list) {
                if (range.conflict(reg_range)) {
                    is_conflict = true;
                    conflict_hard_set.insert(reg);
                    break;
                }
            }
            if (is_conflict)
                break;
        }

        if (is_conflict)
            continue;

        if (occupied_map[reg].empty()) {
            allocated_reg = reg;
            break;
        }

        // nearest successor occupied range
        auto iter = occupied_range_map[reg].lower_bound(alloc_range);
        if (iter != occupied_range_map[reg].begin()) {
            --iter;
        }
        while (iter != occupied_range_map[reg].end()) {
            const auto &occupied_range = iter->first;
            for (const auto &range : range_list) {
                if (range.conflict(occupied_range)) {
                    is_conflict = true;
                    conflict_hard_set.insert(reg);
                    conflict_map[reg].insert(iter->second);
                    break;
                }
            }
            ++iter;
        }

        if (!is_conflict) {
            allocated_reg = reg;
            break;
        }
    }

    if (allocated_reg.has_value()) {
        auto now_reg = allocated_reg.value();
        occupied_map[now_reg].insert(alloc_num);
        for (auto &range : range_list) {
            occupied_range_map[now_reg].insert(AllocRange(range, alloc_num));
        }
        // update other infomathion.
        alloc_map[alloc_num] = now_reg;
        alloc_status_map[alloc_num] = AllocStatus::Assign;

        // TODO: use hint
        if (coalesce_map.count(operand)) {
            auto coalesce_target = coalesce_map[operand];
            hint_map[coalesce_target] = now_reg;
        }
    } else {
        // evict
        auto min_weight = 1e308;
        GReg min_weight_reg;

        for (const auto &[reg, conflict_list] : conflict_map) {
            if (conflict_hard_set.count(reg))
                continue;

            double weight = 0.0;
            for (auto conflict_id : conflict_list) {
                weight += get_spill_weight(conflict_id);
            }

            if (weight < min_weight) {
                min_weight = weight;
                min_weight_reg = reg;
            }
        }

        auto alloc_weight = get_spill_weight(alloc_num);

        if (alloc_weight <= min_weight) {
            // Split
            alloc_status_map[alloc_num] = AllocStatus::Split;
            alloc_priority_queue.emplace(get_alloc_priority(alloc_num));
        } else {
            // Evict the origin min_weight_reg.
            for (auto conflict_num : conflict_map[min_weight_reg]) {
                // update status
                occupied_map[min_weight_reg].erase(conflict_num);
                auto& min_map = occupied_range_map[min_weight_reg];
                for (auto iter = min_map.begin(); iter != min_map.end(); ) {
                    if (iter->second == conflict_num) {
                        occupied_range_map[min_weight_reg].erase(iter);
                        break;
                    }
                    ++iter;
                }

                alloc_map.erase(conflict_num);
                alloc_priority_queue.emplace(get_alloc_priority(conflict_num));
            }

            // update min_weight_reg with alloc_num.
            occupied_map[min_weight_reg].insert(alloc_num);
            for (auto &range : range_list) {
                occupied_range_map[min_weight_reg].emplace(range, alloc_num);
            }
            alloc_map[alloc_num] = min_weight_reg;
        }
    }
}

void Func::try_split(int alloc_num) {
    // simple split
    alloc_status_map[alloc_num] = AllocStatus::Spill;
    alloc_priority_queue.emplace(get_alloc_priority(alloc_num));
}

void Func::spill(int alloc_num) {
    auto operand = alloc_operand_map[alloc_num];
    bool is_flt = operand.index();
    auto range_list = alloc_range_map[alloc_num];
    std::vector<MachineInstr*> def_worklist, use_worklist;
    for (auto&& range : range_list) {
        for (auto instr_num = range.fromNum; instr_num <= range.toNum; ++instr_num) {
            auto instr = num2instr[instr_num];
            auto def = instr->def();
            auto use = instr->use();
            if (std::find(def.begin(), def.end(), operand) != def.end()) {
                def_worklist.push_back(instr);
            }
            if (std::find(use.begin(), use.end(), operand) != use.end()) {
                use_worklist.push_back(instr);
            }
        }
    }
    size_t index;
    if (operand_spill_map.count(operand)) {
        index = operand_spill_map[operand];
    } else {
        operand_spill_map[operand] = index = frame.push(8);
    }

    Map<MachineInstr*, std::pair<std::list<MachineInstr>*, std::list<MachineInstr>::iterator>> lookup;
    for (auto&& block : blocks) {
        for (auto it = block.body.begin(); it != block.body.end(); ++it) {
            lookup[&*it] = {&block.body, it};
        }
    }
    GReg t1; if (is_flt) t1 = FReg::FT1; else t1 = Reg::T1;
    GReg t2; if (is_flt) t2 = FReg::FT2; else t2 = Reg::T2;

    for (auto&& instr : def_worklist) {
        auto [list, it] = lookup[instr];
        GReg reg = t1;
        auto type = is_flt ? LSType::FLOAT : LSType::DWORD;
        instr->replace_def(operand, reg);
        it = list->insert(++it, { LoadStackAddressInstr { Reg::T0, index } });
        it = list->insert(++it, { StoreInstr { type, reg, 0, Reg::T0 } });
    }

    for (auto&& instr : use_worklist) {
        auto [list, it] = lookup[instr];
        GReg reg = t1;
        auto type = is_flt ? LSType::FLOAT : LSType::DWORD;
        if (used_temp_map.count(instr)) {
            reg = t2;
        } else {
            used_temp_map.insert(instr);
        }
        instr->replace_use(operand, reg);
        it = list->insert(it, { LoadStackAddressInstr { Reg::T0, index } });
        it = list->insert(++it, { LoadInstr { type, reg, 0, Reg::T0 } });
    }

}

void Func::rewrite_operands() {
    for (auto [alloc_num, reg] : alloc_map) {
        auto operand = alloc_operand_map[alloc_num];
        auto range_list = alloc_range_map[alloc_num];

        for (const auto &range : range_list) {
            for (auto instr_num = range.fromNum; instr_num <= range.toNum; ++instr_num) {
                auto instr = num2instr[instr_num];

                instr->replace_def(operand, reg);
                instr->replace_use(operand, reg);
            }
        }
        add_saved_register(reg);
    }
}

void Func::add_saved_register(GReg reg) {
    if (getUsage(reg) == RegisterUsage::CalleeSaved && !saved_registers.count(reg)) {
        saved_registers[reg] = frame.push(8);
    }
}

}
