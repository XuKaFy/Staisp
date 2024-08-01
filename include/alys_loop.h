

#pragma once
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_val.h"
namespace Alys {

struct NaturalLoopBody {
    Ir::Block *header;
    Set<Ir::Block *> loop_blocks;
    Ir::Val *ind;
    NaturalLoopBody() = delete;
    NaturalLoopBody(Ir::Block *header, Ir::Block *latch,
                    const Map<Ir::Block *, Set<Ir::Block *>> &dom_set)
        : header(header) {
        complete_loop(latch, dom_set);
    }
    void complete_loop(Ir::Block *latch,
                       const Map<Ir::Block *, Set<Ir::Block *>> &dom_set);
    void handle_indvar(const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) &;
};

using pNaturalLoopBody = std::shared_ptr<NaturalLoopBody>;
[[nodiscard]] pNaturalLoopBody
make_natural_loop(Ir::Block *header, Ir::Block *latch,
                  const Map<Ir::Block *, Set<Ir::Block *>> &dom_set);

struct LoopInfo {
public:
    Map<Ir::Block *, pNaturalLoopBody> loops;

    LoopInfo() = delete;
    LoopInfo(Ir::BlockedProgram &p, const DomTree &dom_ctx);

    NaturalLoopBody *get_loop(Ir::Block *header) const {
        if (loops.count(header) == 0)
            return nullptr;
        return loops.at(header).get();
    }
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