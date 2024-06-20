#include "reg2mem.h"

#include <ir_mem_instr.h>
#include <ir_phi_instr.h>

void reg2mem(Ir::BlockedProgram& program) {
    for (auto&& block : program) {
        std::vector<Ir::pInstr> self;
        for (auto it = block->begin(); it != block->end(); ++it) {
            if (auto phi = dynamic_cast<Ir::PhiInstr*>(it->get())) {
                auto alloc = Ir::make_alloc_instr(phi->ty);
                program.front()->push_behind_end(alloc);
                for (auto [label, val] : *phi) {
                    auto store = Ir::make_store_instr(alloc.get(), val);
                    if (label->block() == block.get()) {
                        self.push_back(store);
                    } else {
                        label->block()->push_behind_end(store);
                    }
                }
                auto load = Ir::make_load_instr(alloc);
                phi->replace_self(load.get());
                it = block->erase(it);
                it = block->insert(it, load);
            }
        }
        for (auto&& store : self) {
            block->push_behind_end(store);
        }
    }
}
