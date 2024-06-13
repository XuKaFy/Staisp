#include "opt.h"

#include "ir_func_defined.h"
#include "opt_1.h"
#include "opt_2.h"
#include "trans_SSA.h"

#include "ir_instr.h"
#include "ir_call_instr.h"
#include "ir_mem_instr.h"

#define MAX_OPT_COUNT 10

namespace Optimize {

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p);

template <typename BlockValue, typename Utils>
int from_bottom_analysis(Ir::BlockedProgram &p);

void func_inline(Ir::pFuncDefined func, AstToIr::Convertor &convertor)
{
    // some function is generated manually, so don't need to be inlined
    if(!func->ast_root)
        return ;
    // printf("try to inline %s\n", func->name().c_str());
    Vector<Ir::CallInstr*> calls;
    for(const auto &use : func->users) {
        auto user = use->user;
        if (user->type() == Ir::VAL_INSTR && 
            dynamic_cast<Ir::Instr*>(user)->instr_type() == Ir::INSTR_CALL) {
            Ir::CallInstr* call_instr = dynamic_cast<Ir::CallInstr*>(user);
            calls.push_back(call_instr);
            // printf("user: %s\n", call_instr->instr_print().c_str());
        }
    }
    for(auto call_instr : calls) {
         /*
        before:
            OriginalBlock:
                ...front...
                [%n = ] call (ty) %func (args...)
                ...back...
        after:
            FrontBlock:
                ...
                (if not void) %m = alloca (ty)
                goto BodyBlock
            (new) BodyBlock:
                 ...
                 all ret modified to:
                * (if not void) store (ret_value), %m
                * br BackBlock
            (new) BackBlock:
                (if not void) %n = load (ty) %m
                ...
        */
        Ir::BlockedProgram* fun = call_instr->block->program;

        if(&func->p == fun) // don't inline self
            continue;

        // printf("***REPLACE %s\n", call_instr->instr_print().c_str());

        Ir::Block* frontBlock = call_instr->block;
        Ir::pBlock backBlock = fun->make_block();
        backBlock->push_back(Ir::make_label_instr());

        Ir::pInstr alloca_instr;
        if (call_instr->ty->type_type() != TYPE_VOID_TYPE)
            alloca_instr = Ir::make_alloc_instr(call_instr->ty);

        // Step 1: copy the will-inline function and replace
        //     all arguments with new arguments
        Ir::pFuncDefined new_fun = convertor.generate_inline_function(func->ast_root);
        Ir::BlockedProgram new_p = new_fun->p;
        for (size_t i = 1; i < call_instr->operand_size(); ++i) {
            new_p.params[i-1]->replace_self(call_instr->operand(i)->usee);
        }
        // Step 2: split original block where CallInstr exists
        auto call_instr_at = frontBlock->body.begin();
        for (; call_instr_at != frontBlock->body.end(); ++call_instr_at) {
            if (call_instr_at->get() == call_instr) {
                break;
            }
        }
        Ir::pInstr call_instr_saver = *call_instr_at;
        for (auto i = std::next(call_instr_at); i != frontBlock->body.end(); ++i) {
            backBlock->push_back(*i);
            (*i)->block = backBlock.get();
        }
        frontBlock->body.erase(call_instr_at, frontBlock->body.end());
        // Step 3: change call to br
        if (alloca_instr)
            fun->blocks.front()->push_behind_end(alloca_instr);
        frontBlock->push_back(make_br_instr(new_p.blocks.front()->label()));
        // Step 4: replace all ret in copied program to two statement:
        // 1. (if not void) store my value to 
        // 2. jump to BackBlock
        for (auto i : new_p.blocks) {
            if (i->back()->instr_type() == Ir::INSTR_RET) {
                Ir::pInstr ret_instr = i->back();
                i->body.pop_back();
                if (alloca_instr) {
                    // printf("new alloca type = %s\n", alloca_instr->ty->type_name().c_str());
                    i->push_back(Ir::make_store_instr(alloca_instr.get(),
                                      ret_instr->operand(0)->usee));
                    // printf("generated store: %s\n", i->body.back()->instr_print().c_str());
                }
                i->push_back(make_br_instr(backBlock->label()));
            }
        }
        // Step 5: load ret value (if exists)
        if (alloca_instr) {
            auto load_instr = make_load_instr(alloca_instr.get());
            backBlock->push_after_label(load_instr);
            call_instr->replace_self(load_instr.get());
            load_instr->block = backBlock.get();
        }
        // Step 5.99: move old alloca to first block
        for (auto i = new_p.blocks.front()->body.begin(); i != new_p.blocks.front()->body.end(); ) {
            if ((*i)->instr_type() == Ir::INSTR_ALLOCA) {
                fun->blocks.front()->push_after_label(*i);
                i = new_p.blocks.front()->body.erase(i);
            } else ++i;
        }
        // Step 6: add new blocks to original function
        for (auto i : new_p.blocks) {
            fun->blocks.push_back(i);
            i->program = fun;
        }
        fun->blocks.push_back(backBlock);
        // Step 7: move imms to original function
        for (auto i : new_p.imms) {
            fun->add_imm(i);
        }
    }
}

void inline_all_function(const Ir::pModule &mod, AstToIr::Convertor &convertor)
{
    // inline all function that can be inlined
    for (auto &&i : mod->funsDefined) {
        func_inline(i, convertor);
    }
    // remove functions that be all inlined
    mod->remove_unused_function();
    // might be inlined so re_generate
    for (auto &&i : mod->funsDefined) {
        i->p.normal_opt();
        i->p.re_generate();
    }
}

void optimize(const Ir::pModule &mod, AstToIr::Convertor &convertor) {
    inline_all_function(mod, convertor);
    for (auto &&i : mod->funsDefined) {
        SSA_pass pass(i->p, ssa_type::RECONSTRUCTION);
        pass.pass_transform();
        // i->print_func()
        /*int cnt = 0;
        for (int opt_cnt = 1; cnt < MAX_OPT_COUNT && (opt_cnt != 0); ++cnt) {
            opt_cnt = from_bottom_analysis<Opt2::BlockValue,
            Opt2::Utils>(i->p); i->p.normal_opt(); opt_cnt +=
            from_top_analysis<Opt1::BlockValue, Opt1::Utils>(i->p);
            i->p.normal_opt();
        }*/
        // // printf("Optimization loop count of function \"%s\": %lu\n",
        // i->name().c_str(), cnt);
        i->p.re_generate();
    }
}

template <typename BlockValue, typename Utils>
int from_top_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = pending_blocks.front();
        pending_blocks.pop_front();

        BlockValue old_OUT = OUTs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        auto in_block = b->in_blocks();
        IN.clear();
        if (!in_block.empty()) {
            IN = OUTs[*in_block.begin()];
            for (auto block : in_block) {
                IN.cup(OUTs[block]);
            }
        }

        OUT = IN;
        util(b, OUT); // transfer function

        if (old_OUT != OUT) {
            auto out_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), out_block.begin(), out_block.end());
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

template <typename BlockValue, typename Utils>
int from_bottom_analysis(Ir::BlockedProgram &p) {
    Map<Ir::Block *, BlockValue> INs;
    Map<Ir::Block *, BlockValue> OUTs;
    std::deque<Ir::Block *> pending_blocks;
    Utils util;
    int ans = 0;
    for (const auto &i : p.blocks) {
        INs[i.get()] = BlockValue();
        OUTs[i.get()] = BlockValue();
        pending_blocks.push_back(i.get());
    }
    while (!pending_blocks.empty()) {
        Ir::Block *b = *pending_blocks.begin();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks();
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        util(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks();
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }
    for (const auto &i : p.blocks) {
        ans += util(i.get(), INs[i.get()], OUTs[i.get()]);
    }
    return ans;
}

} // namespace Optimize
