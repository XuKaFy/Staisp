#include "opt_interprocedural.h"

#include "def.h"
#include "ir_block.h"
#include "ir_func_defined.h"

#include "ir_instr.h"
#include "ir_call_instr.h"
#include "ir_mem_instr.h"
#include "type.h"
#include "value.h"

namespace Optimize {

// 将一个 CallInstr 删除，并且 inline
void replace_call_with_inline(Ir::CallInstr* call_instr)
{
    auto func_inlined = dynamic_cast<Ir::FuncDefined*>(call_instr->operand(0)->usee);
    my_assert(func_inlined, "?");
    auto new_p = func_inlined->p.clone();
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
    Ir::BlockedProgram* fun = call_instr->block()->program();

    // printf("***REPLACE %s\n", call_instr->instr_print().c_str());

    Ir::Block* frontBlock = call_instr->block();
    Ir::pBlock backBlock = Ir::make_block();
    backBlock->push_back(Ir::make_label_instr());

    Ir::pInstr alloca_instr;
    if (call_instr->ty->type_type() != TYPE_VOID_TYPE)
        alloca_instr = Ir::make_alloc_instr(call_instr->ty);

    // Step 1: copy the will-inline function and replace
    //     all arguments with new arguments
    for (size_t i = 1; i < call_instr->operand_size(); ++i) {
        new_p.params()[i-1]->replace_self(call_instr->operand(i)->usee);
    }
    // Step 2: split original block where CallInstr exists
    auto call_instr_at = frontBlock->begin();
    for (; call_instr_at != frontBlock->end(); ++call_instr_at) {
        if (call_instr_at->get() == call_instr) {
            break;
        }
    }
    Ir::pInstr call_instr_saver = *call_instr_at;
    for (auto i = std::next(call_instr_at); i != frontBlock->end(); ++i) {
        backBlock->push_back(*i);
    }
    frontBlock->erase(call_instr_at, frontBlock->end());
    // Step 3: change call to br
    if (alloca_instr)
        fun->front()->push_behind_end(alloca_instr);
    frontBlock->push_back(make_br_instr(new_p.front()->label().get()));
    // Step 4: replace all ret in copied program to two statement:
    // 1. (if not void) store my value to
    // 2. jump to BackBlock
    for (auto&& i : new_p) {
        if (i->back()->instr_type() == Ir::INSTR_RET) {
            Ir::pInstr ret_instr = i->back();
            i->pop_back();
            if (alloca_instr) {
                // printf("new alloca type = %s\n", alloca_instr->ty->type_name().c_str());
                i->push_back(Ir::make_store_instr(alloca_instr.get(),
                                  ret_instr->operand(0)->usee));
                // printf("generated store: %s\n", i->body.back()->instr_print().c_str());
            }
            i->push_back(make_br_instr(backBlock->label().get()));
        }
    }
    // Step 5: load ret value (if exists)
    if (alloca_instr) {
        auto load_instr = make_load_instr(alloca_instr.get());
        backBlock->push_after_label(load_instr);
        call_instr->replace_self(load_instr.get());
    }
    // Step 5.99: move old alloca to first block
    for (auto i = new_p.front()->begin(); i != new_p.front()->end(); ) {
        if ((*i)->instr_type() == Ir::INSTR_ALLOCA) {
            fun->front()->push_after_label(*i);
            i = new_p.front()->erase(i);
        } else ++i;
    }
    // Step 6: add new blocks to original function
    for (auto i : new_p) {
        fun->push_back(i);
    }
    fun->push_back(backBlock);
    // Step 7: move imms to original function
    fun->cpool.merge(new_p.cpool);
    // Step 8: clear new_p
    new_p.clear();
}

// 判断一个函数是否应该被内联进其他函数
bool should_function_be_inlined(const Ir::pFuncDefined &f)
{
    if (f->name() == BUILTIN_INITIALIZER) {
        // BUILTIN_INITIALIZER should NOT be inlined
        return false;
    }
    for (auto&& block : f->p) {
        for (auto&& instr : *block) {
            if (instr->instr_type() != Ir::INSTR_CALL) continue;
            auto call_instr = dynamic_cast<Ir::CallInstr*>(instr.get());
            if (auto callee = dynamic_cast<Ir::FuncDefined*>(call_instr->operand(0)->usee); 
                callee != nullptr) {
                // when a function calls a user-defined function (**INCLUDE ITSELF**)
                // it should NOT be inlined into other functions 
                return false;
            }
        }
    }
    return true;
}

// 判断某个 CallInstr 所在的地方是否该进行内联
bool should_call_instr_replaced_by_inline(const Ir::CallInstr* call_instr)
{
    for (auto&& use : call_instr->users()) {
        if (use->user->type() != Ir::VAL_INSTR) continue;
        auto user_instr = dynamic_cast<Ir::Instr*>(use->user);
        switch (user_instr->instr_type()) {
        case Ir::INSTR_MINI_GEP:
        case Ir::INSTR_ITEM:
            // if a pure function call is used by a GEP
            // it should NOT inline this callee function
            return false;
        default: break;
        }
    }
    return true;
}

// 遍历所有应该 inline 的函数，并且尝试 inline
int inline_all_function(const Ir::pModule &mod)
{
    int inline_cnt = 0;

    // inline all function that can be inlined
    for (auto&& func : mod->funsDefined) {
        if (!should_function_be_inlined(func)) continue;
        
        Vector<Ir::CallInstr*> ready_to_inline;
        for (auto &&use : func->users()) {
            if (use->user->type() != Ir::VAL_INSTR) continue;
            auto user_instr = dynamic_cast<Ir::Instr*>(use->user);
            if (user_instr->instr_type() != Ir::INSTR_CALL) continue;
            auto call_instr = dynamic_cast<Ir::CallInstr*>(user_instr);
            if (should_call_instr_replaced_by_inline(call_instr)) {
                // should NOT inline here
                // because func->users() changed
                ready_to_inline.push_back(call_instr);
            }
        }
        
        for (auto &&call_instr : ready_to_inline) {
            replace_call_with_inline(call_instr);
            inline_cnt += 1;
        }
    }
    
    // plain opt all functions
    if (inline_cnt) {
        for (auto &&i : mod->funsDefined) {
            i->p.plain_opt_all();
        }
    }

    mod->remove_unused_function();
    return inline_cnt;
}

Ir::BlockedProgram* is_used_by_single_function(const Ir::pGlobal &p)
{
    Ir::BlockedProgram *func = nullptr;
    if (!is_basic_type(p->con.v.ty)) {
        return nullptr;
    }
    for (auto &&j : p->users()) {
        my_assert(j->user->type() == Ir::VAL_INSTR, "?");
        auto user_instr = dynamic_cast<Ir::Instr*>(j->user);
        auto user_program = user_instr->block()->program();
        if (user_program->name() == BUILTIN_INITIALIZER)
            continue;
        if (func == nullptr) {
            func = user_program;
        } else if (user_program != func) {
            return nullptr;
        }
    }
    return func;
}

void global2local(const Ir::pModule &mod)
{
    // warning: only inline globals that only used in function "main"
    // may cause stack overflow !!!

    mod->remove_unused_global();

    for (auto i = mod->globs.begin(); i != mod->globs.end(); ) {
        Ir::BlockedProgram *func = is_used_by_single_function(*i);
        if (func == nullptr) {
            ++i;
            continue;
        }
        if (func->name() != "main" && !func->is_pure()) {
            ++i;
            // must be a pure function
            continue;
        }
        // Step 1: make a new alloca
        auto alloca_instr = Ir::make_alloc_instr(to_pointed_type((*i)->ty));
        // Step 2: replace global with this alloca
        // for array, because initializer will initialize this value
        // so just replace it will make sense
        (*i)->replace_self(alloca_instr.get());
        // Step 3: for imm value, store the value
        auto store_instr = Ir::make_store_instr(alloca_instr.get(),
                           func->cpool.add((*i)->con.v).get());
        // Step 4: insert
        func->front()->push_after_label(alloca_instr);
        if (store_instr) {
            for (auto j = std::next(func->front()->begin()); j != func->front()->end(); ++j) {
                if ((*j)->instr_type() != Ir::INSTR_ALLOCA) {
                    func->front()->insert(j, store_instr);
                    break;
                }
            }
        }
        // Step 5
        i = mod->globs.erase(i);
    }
}

}