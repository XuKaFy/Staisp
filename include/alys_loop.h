

#pragma once
#include "alys_dom.h"
#include "def.h"
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
    void print_loop() const;
};

// a is dominated by b
inline bool is_dom(Ir::Block *a, Ir::Block *b,
                   const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    return dom_set.at(a).count(b) > 0;
}
// dom context for dominator relation
auto build_dom_set(const DomTree &dom_ctx)
    -> Map<Ir::Block *, Set<Ir::Block *>>;

} // namespace Alys