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

void func_inline_from_bp(Ir::CallInstr* call_instr, Ir::BlockedProgram &new_p)
{
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
    frontBlock->push_back(make_br_instr(new_p.front()->label()));
    // Step 4: replace all ret in copied program to two statement:
    // 1. (if not void) store my value to
    // 2. jump to BackBlock
    for (auto i : new_p) {
        if (i->back()->instr_type() == Ir::INSTR_RET) {
            Ir::pInstr ret_instr = i->back();
            i->pop_back();
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

bool func_inline(Ir::pFuncDefined func, AstToIr::Convertor &convertor)
{
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

    if (calls.size() == 1 && !func->ast_root) { // __buildin_initializer has no ast_root
        // printf("%s move codes\n", func->name().c_str());
        func_inline_from_bp(calls.front(), func->p); // delete self
        return true;
    }

    for(auto call_instr : calls) {
        Ir::BlockedProgram* fun = call_instr->block()->program();
        if (&func->p == fun) // don't inline self
            continue;

        Ir::pFuncDefined new_fun = convertor.generate_inline_function(func->ast_root);
        Ir::BlockedProgram new_p = new_fun->p;

        func_inline_from_bp(call_instr, new_p);
    }
    return false;
}

void inline_all_function(const Ir::pModule &mod, AstToIr::Convertor &convertor)
{
    // remove functions that be all inlined
    mod->remove_unused_function();
    // inline all function that can be inlined
    for (auto i = mod->funsDefined.begin(); i != mod->funsDefined.end();) {
        if (func_inline(*i, convertor)) {
            i = mod->funsDefined.erase(i);
        } else ++i;
    }
    // remove functions that be all inlined
    mod->remove_unused_function();
    // might be inlined so re_generate
    for (auto &&i : mod->funsDefined) {
        i->p.plain_opt_all();
    }
}

void global2local(const Ir::pModule &mod)
{
    Set<Ir::BlockedProgram*> mod_funcs;
    for (auto i = mod->globs.begin(); i != mod->globs.end(); ) {
        bool onlyfans = true;
        Ir::BlockedProgram *func = nullptr;
        if ((*i)->users.empty()) {
            i = mod->globs.erase(i);
            continue;
        }
        if (!is_basic_type((*i)->con.v.ty)) {
            ++i;
            continue;
        }
        for (auto j : (*i)->users) {
            my_assert(j->user->type() == Ir::VAL_INSTR, "?");
            auto user_instr = dynamic_cast<Ir::Instr*>(j->user);
            if (func == nullptr) func = user_instr->block()->program();
            if (user_instr->block()->program() != func) {
                onlyfans = false;
                break;
            }
        }
        if (!onlyfans) {
            ++i;
            continue;
        }
        mod_funcs.insert(func);
        // Step 1: make a new alloca
        auto alloca_instr = Ir::make_alloc_instr(to_pointed_type((*i)->ty));
        // Step 2: replace global with this alloca
        // for array, because initializer will initialize this value
        // so just replace it will make sense
        (*i)->replace_self(alloca_instr.get());
        // Step 3: for imm value, store the value
        auto store_instr = Ir::make_store_instr(alloca_instr,
                           func->cpool.add((*i)->con.v));
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