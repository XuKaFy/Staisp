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
                [](auto&) { return nullptr; }
            }, instr.instr);
            if (to != nullptr) {
                block.out_blocks.push_back(to);
                to->in_blocks.push_back(&block);
            }
        }
    }
}


} // namespace Backend
