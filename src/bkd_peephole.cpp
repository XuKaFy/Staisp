#include <bkd_func.h>

namespace Backend {

bool peephole_mv_self(std::deque<Block>& blocks) {
    bool changed = false;
    for (auto&& block : blocks) {
        for (auto it = block.body.begin(); it != block.body.end(); ) {
            if (it->instr_type() == MachineInstr::Type::REG) {
                auto [type, rd, rs] = it->as<RegInstr>();
                if (type == RegInstrType::MV && rd == rs) {
                    it = block.body.erase(it);
                    changed = true;
                    continue; // skip ++it
                }
            }
            if (it->instr_type() == MachineInstr::Type::FREG) {
                auto [type, rd, rs] = it->as<FRegInstr>();
                if (type == FRegInstrType::FMV_S && rd == rs) {
                    it = block.body.erase(it);
                    changed = true;
                    continue; // skip ++it
                }
            }
            ++it; // don't move it elsewhere
        }
    }
    return changed;
}

bool peephole_remove_j(std::deque<Block>& blocks) {
    bool changed = false;
    for (size_t i = 0; i + 1 < blocks.size(); ++i) {
        auto& this_block = blocks[i];
        auto& next_block = blocks[i + 1];
        if (this_block.body.back().instr_type() == MachineInstr::Type::J) {
            if (this_block.body.back().as<JInstr>().label == next_block.name) {
                this_block.body.pop_back();
                changed = true;
            }
        }
    }
    return changed;
}

bool Func::peephole() {
    bool changed = false;
    changed |= peephole_mv_self(blocks);
    changed |= peephole_remove_j(blocks);
    return changed;
}


}