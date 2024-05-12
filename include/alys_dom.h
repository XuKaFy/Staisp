
#pragma once
#include "def.h"
#include "ir_block.h"
namespace Alys {
struct DomBlock;

using pDomBlock = Pointer<DomBlock>;

struct DomBlock {
    Set<DomBlock *> out_block;
    DomBlock *idom;
    Ir::pBlock basic_block;
};

struct DomTree {
    Map<Ir::Block *, pDomBlock> dom_map;
    pDomBlock make_domblk();
    void print_dom_tree() const;
    void build_dom(Ir::BlockedProgram &p);
    Map<Ir::Block *, pDomBlock> build_dom_frontier() const;
};

}; // namespace Alys