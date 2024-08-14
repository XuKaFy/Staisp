#pragma once

#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_val.h"
#include <ir_cmp_instr.h>

namespace Alys {

struct NaturalLoopBody {
    Ir::Block *header;
    Set<Ir::Block *> loop_blocks;
    Ir::Val *ind;
    Ir::Val *bound;
    Ir::CmpType cmp_op;
    Ir::CmpInstr *loop_cnd_instr;

    NaturalLoopBody() = delete;
    NaturalLoopBody(Ir::Block *header, Ir::Block *latch,
                    const Map<Ir::Block *, Set<Ir::Block *>> &dom_set)
        : header(header) {
        complete_loop(latch, dom_set);
    }
    void complete_loop(Ir::Block *latch,
                       const Map<Ir::Block *, Set<Ir::Block *>> &dom_set);
    void handle_indvar(const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) &;

    std::string print();
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

} // namespace Alys