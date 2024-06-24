

#pragma once
#include "alys_dom.h"
#include "ir_block.h"
namespace Alys {

struct NaturalLoop {
    Ir::Block *header;
    Ir::Block *latch;
    Set<Ir::Block *> loop_blocks;
    NaturalLoop() = delete;
    NaturalLoop(Ir::Block *header, Ir::Block *latch)
        : header(header), latch(latch) {
        complete_loop();
    }
    void complete_loop();
};

using pNaturalLoop = std::shared_ptr<NaturalLoop>;
[[nodiscard]] pNaturalLoop make_natural_loop(Ir::Block *header,
                                             Ir::Block *latch);

struct LoopInfo {
    Vector<pNaturalLoop> loops;

    LoopInfo() = delete;
    LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx);
    // a is dominated by b
    static inline bool
    is_dom(Ir::Block *a, Ir::Block *b,
           const Map<Ir::Block *, Set<Ir::Block *>> &dom_set);
    // dom context for dominator relation
    auto build_dom_set(Ir::Block *bb, Set<Ir::Block *> idom_dom_set) -> void;
    void print_loop() const;
};
} // namespace Alys