#include <bkd_func.h>

namespace Backend {

bool peephole_mv_self(std::deque<Block>& blocks) {
    bool changed = false;
    for (auto&& block : blocks) {
        for (auto it = block.body.begin(); it != block.body.end(); ) {
            if (it->instr_type() == MachineInstr::Type::REG) {
                auto& instr = it->as<RegInstr>();
                if (instr.type == RegInstrType::MV && instr.rd == instr.rs) {
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

bool Func::peephole() {
    bool changed = false;
    changed |= peephole_mv_self(blocks);
    return changed;
}


}