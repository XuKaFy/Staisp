
#pragma once
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"

namespace Alys {
struct DomBlock;

using pDomBlock = Pointer<DomBlock>;
using DomPredicate = Map<Ir::Block *, Set<Ir::Block *>>;

struct DomBlock {
    Vector<DomBlock *> out_block;
    DomBlock *idom;
    Ir::Block *basic_block;
};

// a is dominated by b
inline bool is_dom(Ir::Block *a, Ir::Block *b,
                   const Map<Ir::Block *, Set<Ir::Block *>> &dom_set) {
    return dom_set.at(a).count(b) > 0;
}
struct DomTree {
    Map<Ir::Block *, pDomBlock> dom_map;
    static auto make_domblk() -> pDomBlock;
    void print_dom_tree() const;
    void build_dom(Ir::BlockedProgram &p);
    void build_dfsn(Set<DomBlock *> &v, DomBlock *cur);
    [[nodiscard]] Map<Ir::Block *, pDomBlock> build_dom_frontier() const;

    Vector<Ir::Block *> dom_order;
    DomPredicate dom_set;
    Set<Ir::Block *> unreachable_blocks;
    const Vector<Ir::Block *> &order() const { return dom_order; }
    Ir::Block *LCA(Ir::Block *, Ir::Block *);
    // a is dominated by b
    inline bool is_dom(Ir::Block *a, Ir::Block *b) const {
        return ::Alys::is_dom(a, b, dom_set);
    }
};

// dom context for dominator relation
DomPredicate build_dom_set(const DomTree &dom_ctx);
}; // namespace Alys
