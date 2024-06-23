

#pragma once
#include "alys_dom.h"
#include "ir_block.h"
#include <sys/stat.h>

namespace Alys {

struct NaturalLoop {
    Ir::Block *header;
    Ir::Block *latch;
    Set<Ir::Block *> loop_blocks;
};

using pNaturalLoop = std::shared_ptr<NaturalLoop>;
[[nodiscard]] pNaturalLoop make_natural_loop();

struct LoopInfo {
    Vector<pNaturalLoop> loops;

    LoopInfo() = delete;
    LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx);
    // a is dominated by b
    static inline bool
    is_dom(Ir::Block *a, Ir::Block *b,
           const Map<Ir::Block *, Set<Ir::Block *>> &dom_set);
    void print_loop() const;
};
} // namespace Alys