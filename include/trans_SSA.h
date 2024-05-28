#pragma once
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_instr.h"
#include "ir_val.h"

#include <functional>

namespace Optimize {

enum class ssa_type { LLVM_MEM2REG, RECONSTRUCTION };
class SSA_pass {

    ssa_type promotion_type;
    using vrtl_reg = Ir::Val;
    Map<vrtl_reg *, Map<Ir::Block *, vrtl_reg *>> current_def;
    Set<Ir::Block *> sealedBlocks;
    Ir::BlockedProgram &cur_func;
    Alys::DomTree dom_ctx;
    Map<Ir::Block *, Vector<Pair<vrtl_reg *, Ir::Instr *>>> incompletePhis;

public:
    SSA_pass(Ir::BlockedProgram &arg_function, const ssa_type &arg_ssa_type);

    auto entry_blk() -> Ir::Block *;

    auto def_val(vrtl_reg *variable, Ir::Block *block, vrtl_reg *val) -> void;

    template <typename a, typename b>
    auto fmap(std::function<b(a)> f, Vector<a> &v1) -> Vector<b> {
        Vector<b> v2;
        for (const auto &vs : v1) {
            v2.push_back(f(vs));
        }
        return v2;
    };

    static auto is_phi(Ir::User *user) -> bool;

    vrtl_reg *use_val(vrtl_reg *variable, Ir::Block *block);

    vrtl_reg *use_val_recursive(vrtl_reg *variable, Ir::Block *block);

    vrtl_reg *addPhiOperands(vrtl_reg *variable, Ir::Instr *phi,
                             Ir::Block *phi_blk);

    auto tryRemoveTrivialPhi(Ir::PhiInstr *phi) -> vrtl_reg *;

    void sealBlock(Ir::Block *block);
    auto unreachable_blks() -> Set<Ir::Block *>;
    void pass_transform();
    void reconstruct();
};
} // namespace Optimize