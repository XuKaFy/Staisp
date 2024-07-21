#include "bkd_func.h"

namespace Backend {

String Func::generate_asm() const
{
    String res;

    res += name;
    res += ":\n";

    for (auto &&block : blocks) {
        res += block.print();
    }

    return res;
}

void Func::build_block_graph() {
    std::unordered_map<std::string, Block*> name2block;
    for (auto&& block : blocks) {
        name2block[block.name] = &block;
    }
    for (auto&& block : blocks) {
        for (auto&& instr : block.body) {
            Block* to = std::visit(overloaded {
                [&](JInstr& j) { return name2block[j.label]; },
                [&](BranchInstr& j) { return name2block[j.label]; },
                [&](RegLabelInstr& j) { return name2block[j.label]; },
                [](auto&) -> Block* { return nullptr; }
            }, instr.instr);
            if (to != nullptr) {
                block.out_blocks.push_back(to);
                to->in_blocks.push_back(&block);
            }
        }
    }
}

void Func::build_block_def_use() {
    for (auto&& block : blocks) {
        for (auto&& instr : block.body) {
            auto uses = instr.use();
            for (auto&& use : uses) {
                // use after redef is not considered
                if (block.defs.count(use) == 0) {
                    block.uses.insert(use);
                }
            }
            auto defs = instr.def();
            for (auto&& def : defs) {
                block.defs.insert(def);
            }
        }
    }
}

void Func::number_instruction(Block *block) {
    if (visited.count(block)) return;
    visited.insert(block);
    for (auto&& instr : block->body) {
        instr.number = next_num();
        num2instr[instr.number] = &instr;
    }
    for (auto&& out : block->out_blocks) {
        number_instruction(out);
    }
}

bool BlockValue::operator==(const BlockValue &b) const {
    return uses == b.uses;
}

bool BlockValue::operator!=(const BlockValue &b) const {
    return !operator==(b);
}

void BlockValue::cup(const BlockValue &v) {
    for (const auto &i : v.uses) {
        uses.insert(i);
    }
}

void BlockValue::clear() {
    uses.clear();
}

void transfer(const Block *block, BlockValue &in) {
    // in = use + (in - def)

    for (auto i : block->defs) {
        in.uses.erase(i);
    }
    for (auto i : block->uses) {
        in.uses.insert(i);
    }
}

void Func::liveness_analysis() {
    Map<const Block *, BlockValue> INs;
    Map<const Block *, BlockValue> OUTs;
    std::deque<const Block *> pending_blocks;
    for (const auto &block : blocks) {
        INs[&block] = BlockValue();
        OUTs[&block] = BlockValue();
        pending_blocks.push_back(&block);
    }
    while (!pending_blocks.empty()) {
        const Block *b = pending_blocks.front();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue old_OUT = OUTs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks;
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        transfer(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks;
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }

    for (auto& block : blocks) {
        auto frontNum = block.body.front().number;
        auto backNum = block.body.end()->number;
        Map<GReg, LiveRange> range_buffer;
        for (auto&& reg : OUTs[&block].uses) {
            range_buffer[reg] = LiveRange{frontNum, backNum, 0, &block};
        }

        for (auto it = block.body.rbegin(); it != block.body.rend(); ++it) {
            auto&& instr = *it;
            auto curNum = instr.number;
            for (auto reg : it->def()) {
                if (!range_buffer.count(reg)) {
                    // dead def (in current block)
                    live_ranges[reg].push_back({
                        curNum, curNum, 1, &block
                    });
                } else {
                    auto range = range_buffer[reg];
                    range.fromNum = curNum;
                    range.instr_cnt++;
                    live_ranges[reg].push_back(range);
                    range_buffer.erase(reg);
                }
            }
            for (auto reg : it->use()) {
                if (!range_buffer.count(reg)) {
                    // new use
                    range_buffer[reg] = LiveRange{frontNum, curNum, 1, &block};
                } else {
                    range_buffer[reg].instr_cnt++;
                }
            }
        }

        for (auto [reg, range] : range_buffer)
        {
            live_ranges[reg].push_back(range);
        }
    }
    for (auto &[reg, range_list] : live_ranges)
    {
        std::sort(range_list.begin(), range_list.end(), [](const LiveRange&a, const LiveRange&b) { return a.fromNum < b.fromNum; });
    }
}

} // namespace Backend
