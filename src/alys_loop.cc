
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cstdint>
#include <memory>
namespace Alys {

struct LoopHdr {
    Ir::Block *header;
    Ir::Block *latch;

    Ir::Val *next;
};

using pLoopHdr = Pointer<LoopHdr>;

struct loop_list final {
    Vector<pLoopHdr> loops;
    void build_loop(Ir::BlockedProgram &arg_func);
};

void loop_list::build_loop(Ir::BlockedProgram &arg_func) {

    DomTree dom_tree;
    dom_tree.build_dom(arg_func);

    for (auto *cur_blk : dom_tree.order()) {
        auto back_instr = cur_blk->back();
        if (back_instr->instr_type() != Ir::INSTR_BR_COND) {
            continue;
        }

        auto br_instr = std::dynamic_pointer_cast<Ir::BrCondInstr>(back_instr);
        auto cond = br_instr->operands.at(0)->usee;
        auto cmp_cnd = dynamic_cast<Ir::CmpInstr *>(cond);
        if (!cmp_cnd) {
            continue;
        }

        if (!is_integer(cmp_cnd->ty)) {
            continue;
        }

        const auto next = cmp_cnd->operands.at(0)->usee;
        auto bound = cmp_cnd->operands.at(1)->usee;

        Ir::Val *indvar;
        intmax_t step;
    }
}
} // namespace Alys