
#pragma once
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_val.h"
#include "type.h"
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
    static pDomBlock make_domblk();
    void print_dom_tree() const;
    void build_dom(Ir::BlockedProgram &p);
    [[nodiscard]] Map<Ir::Block *, pDomBlock> build_dom_frontier() const;
};

}; // namespace Alys

namespace Ir {
struct PhiInstr final : Instr {
    PhiInstr(const pType &type);

    InstrType instr_type() const override { return INSTR_PHI; }

    String instr_print() const override;

    void add_incoming(Block *blk, Val *val);

    Map<Block *, pUse> incoming_tuples;
};

// make_phi_instr: create a new PhiInstr with the given type
[[nodiscard]] pInstr make_phi_instr(const pType &type);
}; // namespace Ir