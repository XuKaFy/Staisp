#include "opt_array_accumulate.h"
#include "common_node.h"
#include "def.h"
#include "ir_block.h"
#include "ir_cmp_instr.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "ir_phi_instr.h"
#include "ir_ptr_instr.h"
#include "ir_val.h"
#include "type.h"
#include "opt_ptr_iter.h"
#include "value.h"

namespace Optimize {

Vector<ImmValue> getarray(const Ir::AllocInstr* arr, Ir::Block* cur_block, const Alys::DomTree &dom, size_t end)
{
    Vector<ImmValue> ans;
    ans.resize(end + 1);
    Ir::Block* store_at = nullptr;

    // printf("TRY FIND ARRAY %s\n", arr->instr_print().c_str());

    for (auto && use : arr->users()) {
        auto gep_user = dynamic_cast<Ir::MiniGepInstr*>(use->user);
        if (gep_user == nullptr) {
            return {};
        }

        // printf("    FIND GEP %s\n", gep_user->instr_print().c_str());
        for (auto &&use2 : gep_user->users()) {
            auto store_user = dynamic_cast<Ir::StoreInstr*>(use2->user);
            if (store_user == nullptr)
                continue;

            // printf("    FIND STORE %s\n", store_user->instr_print().c_str());

            if (gep_user->operand(1)->usee->type() != Ir::VAL_CONST) {
                // printf("    GEP NOT A CONSTANT\n");
                continue;
                return {};
            }
        
            if (store_user->operand(1)->usee->type() != Ir::VAL_CONST) {
                // printf("    STORE NOT A CONSTANT\n");
                return {};
            }
        
            if (store_at == nullptr) {
                store_at = store_user->block();
                if (!dom.is_dom(cur_block, store_at)) {
                    // printf("    NOT DOMED\n");
                    return {};
                }
            } else if (store_at != store_user->block()) {
                // printf("    HAS MULTIPLY BLOCK\n");
                return {};
            }

            uint64_t index = static_cast<Ir::Const*>(gep_user->operand(1)->usee)->v.imm_value().val.uval;
            ImmValue cur = static_cast<Ir::Const*>(store_user->operand(1)->usee)->v.imm_value();
            // printf("    STORE [%lu] = %s\n", index, cur.print().c_str());
            
            ans[index] = cur;
        }
    }
    return ans;
}

void array_accumulate(const Ir::pFuncDefined& func, 
                      const Alys::DomTree &dom,
                      const Alys::LoopInfo &loop_info)
{
    for (auto&& [_, loop] : loop_info.loops) {
        if (loop->loop_blocks.size() != 2) continue;

        int64_t start, end;
        Ir::Block* pred = nullptr;
        auto iter_info = detect_iteration(loop);

        if (iter_info) {
            if (loop->cmp_op != Ir::CMP_SLT && loop->cmp_op != Ir::CMP_SLE)
                continue;
            if (iter_info->initial->type() != Ir::VAL_CONST)
                continue;

            start = static_cast<Ir::Const*>(iter_info->initial)->v.imm_value().val.ival;
            end = static_cast<Ir::Const*>(loop->bound)->v.imm_value().val.ival;
            if (loop->cmp_op == Ir::CMP_SLT)
                --end;

            pred = iter_info->pred_block;
        } else {
            continue;
        }
        my_assert(pred, "?");
        
        Ir::Block* body = nullptr;

        for (auto&& block : loop->loop_blocks) {
            if (block == loop->header) continue;
            body = block;
        }
        my_assert(body, "?");
        if (body->size() != 6) continue;
        /*
            L9:
                %109 = phi i32 [ %114, %L10 ], [ %105, %L8 ]
                %110 = phi i32 [ %115, %L10 ], [ 0, %L8 ]
                %111 = icmp slt i32 %110, 100
                br i1 %111, label %L10, label %L11
            L10:
                %112 = getelementptr [100 x i32], [100 x i32]* %0, i32 0, i32 %110
                %113 = load i32, i32* %112
                %114 = add i32 %109, %113
                %115 = add i32 %110, 1
                br label %L9
        
            gep arr, ind
            load arr
            add outer_sum, arr
            add ind, 1
        */
        auto i = std::next(body->begin()); // jump label
        auto gep = dynamic_cast<Ir::MiniGepInstr*>((i++)->get());
        auto load = dynamic_cast<Ir::LoadInstr*>((i++)->get());
        auto bin = dynamic_cast<Ir::BinInstr*>((i++)->get());
        auto bin_ind = dynamic_cast<Ir::BinInstr*>((i++)->get());
        
        if (load == nullptr || bin == nullptr || gep == nullptr || bin_ind == nullptr)
            continue;
        if (bin->binType != Ir::INSTR_ADD) // only for accumulation
            continue;
        if (load->operand(0)->usee != gep)
            continue;
        if (loop->ind != gep->operand(1)->usee)
            continue;
        
        // only for 1 dimension
        // and it must NOT be global
        auto arr = dynamic_cast<Ir::AllocInstr*>(gep->operand(0)->usee);
        if (!arr || !is_pointer(arr->ty) 
            || !is_array(to_pointed_type(arr->ty))
            || !is_basic_type(to_elem_type(to_pointed_type(arr->ty)))) continue;

        Ir::Val* outer_sum = nullptr;    
        if (bin->operand(0)->usee == load) {
            outer_sum = bin->operand(1)->usee;
        } else if (bin->operand(1)->usee == load) {
            outer_sum = bin->operand(0)->usee;
        } else continue;
        my_assert(outer_sum, "?");

        if (!is_basic_type(outer_sum->ty))
            continue;
    
        auto arr_val = getarray(arr, body, dom, end);
        if (arr_val.empty()) continue;

        ImmValue accumu (to_basic_type(outer_sum->ty)->ty);
        for (auto i=start; i<=end; ++i) {
            accumu = accumu + arr_val[i];
        }

        auto phi = dynamic_cast<Ir::PhiInstr*>(outer_sum);
        if (phi == nullptr)
            continue;
        // printf("OUTER_SUM = %s\n", phi->instr_print().c_str());

        Ir::Val* initial = nullptr;
        for (auto [label, val] : *phi) {
            if (!iter_info->loop->loop_blocks.count(label->block())) {
                my_assert(initial == nullptr, "?");
                initial = val;
            }
        }
        auto add = Ir::make_binary_instr(Ir::INSTR_ADD, initial, pred->add_imm(accumu).get());
        pred->push_behind_end(add);
        phi->replace_self(add.get());
        // printf("PUSH %s\n", add->instr_print().c_str());

        auto header = loop->header;
        header->squeeze_out(false);
        for (auto it = header->begin(); it != header->end(); ) {
            auto type = (*it)->instr_type();
            if (type == Ir::INSTR_CMP) {
                it = header->erase(it);
            } else {
                ++it;
            }
        }
        for (auto i = header->begin(); i != header->end(); ) {
            if ((*i)->instr_type() == Ir::INSTR_PHI) {
                i = header->erase(i);
            } else {
                ++i;
            }
        }
    }
}

} // namespace Optimize
