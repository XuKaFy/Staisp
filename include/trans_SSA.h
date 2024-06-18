#pragma once
#include "alys_dom.h"
#include "def.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "ir_val.h"

#include "ir_phi_instr.h"

#include <functional>

namespace Optimize {

class SSA_pass {

    using vrtl_reg = Ir::Val;
    Map<vrtl_reg *, Map<Ir::Block *, Ir::pUse>> current_def;
    Set<Ir::Block *> sealedBlocks;
    Ir::BlockedProgram &cur_func;
    Alys::DomTree dom_ctx;
    Map<Ir::Block *, Vector<Pair<vrtl_reg *, Ir::pInstr>>> incompletePhis;

public:
    SSA_pass(Ir::BlockedProgram &arg_function);

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
    void reconstruct();

private:
    Set<vrtl_reg *> function_args;
    Ir::pUse blk_def(Ir::Block *block, vrtl_reg *blk_def_val);
    struct undef_allocator {
        Ir::pVal i32;
        Ir::pVal f32;
        Ir::pVal i1;
    } undefined_values;
    auto erase_blk_def(Ir::pUse &blk_def_use) -> void;
    auto undef_val(vrtl_reg *variable) -> Ir::Const *;
};
} // namespace Optimize