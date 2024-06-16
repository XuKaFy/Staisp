
#pragma once
#include "def.h"
#include "ir_block.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
namespace Alys {
struct DomBlock;

using pDomBlock = Pointer<DomBlock>;

struct DomBlock {
    Vector<DomBlock *> out_block;
    DomBlock *idom;
    Ir::Block *basic_block;
};

struct DomTree {
    Map<Ir::Block *, pDomBlock> dom_map;
    static auto make_domblk() -> pDomBlock;
    void print_dom_tree() const;
    void build_dom(Ir::BlockedProgram &p);
    void build_dfsn(Set<DomBlock *> &v, DomBlock *cur);
    [[nodiscard]] Map<Ir::Block *, pDomBlock> build_dom_frontier() const;

    Vector<Ir::Block *> dom_order;
    Set<Ir::Block *> unreachable_blocks;
    const Vector<Ir::Block *> &order() const { return dom_order; }
};

}; // namespace Alys

namespace Ir {
struct PhiInstr final : Instr {
    PhiInstr(const pType &type);

    InstrType instr_type() const override { return INSTR_PHI; }

    String instr_print() const override;

    void add_incoming(Block *blk, Val *val);
    void add_label(Block *blk);

    //    Map<Block *, pUse> incoming_tuples;
    Vector<Ir::LabelInstr *> labels;
};

// make_phi_instr: create a new PhiInstr with the given type
[[nodiscard]] pInstr make_phi_instr(const pType &type);
}; // namespace Ir