#include "ir_func_defined.h"

#include <alloca.h>
#include <utility>

#include <memory>

#include "ir_block.h"
#include "ir_call_instr.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_val.h"
#include "type.h"

namespace Ir {

FuncDefined::FuncDefined(const TypedSym &var, Vector<pType> arg_types,
                         Vector<String> arg_name)
    : Func(var, arg_types) {
    this->arg_name = arg_name;
    size_t length = arg_types.size();
    add_body(make_label_instr());
    for (size_t i = 0; i < length; ++i) {
        auto sym_node =
            make_sym_instr(TypedSym("%" + arg_name[i], arg_types[i]));
        pInstr alloced = make_alloc_instr(arg_types[i]);
        pInstr stored = make_store_instr(alloced, sym_node);
        add_body(alloced);
        add_body(stored);
        add_imm(sym_node);
        args.push_back(alloced);
    }
}

void FuncDefined::add_body(const pInstr &instr) { body.push_back(instr); }

void FuncDefined::add_imm(const pVal &val) { imms.push_back(val); }

void FuncDefined::inline_self()
{
    for(const auto &use : users) {
        auto user = use->user;
        if (user->type() == VAL_INSTR && 
            dynamic_cast<Instr*>(user)->instr_type() == INSTR_CALL) {
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
            CallInstr* call_instr = dynamic_cast<CallInstr*>(user);
            BlockedProgram* fun = call_instr->block->program;

            Block* frontBlock = call_instr->block;
            pBlock backBlock = fun->make_block();
            backBlock->push_back(make_label_instr());

            pInstr alloca_instr;
            if (call_instr->ty->type_type() != TYPE_VOID_TYPE)
                alloca_instr = make_alloc_instr(call_instr->ty);

            // Step 1: copy the will-inline function and replace
            //     all arguments with new arguments
            BlockedProgram new_p = p.copy_self();
            for (size_t i = 1; i < call_instr->operand_size(); ++i) {
                new_p.args[i-1]->replace_self(call_instr->operand(i)->usee);
            }
            // Step 2: split original block where CallInstr exists
            auto call_instr_at = frontBlock->body.begin();
            for (; call_instr_at != frontBlock->body.end(); ++call_instr_at) {
                if (call_instr_at->get() == call_instr) {
                    break;
                }
            }
            for (auto i = call_instr_at; i != frontBlock->body.end(); ++i) {
                backBlock->push_back(*i);
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
                if (i->back()->instr_type() == INSTR_RET) {
                    pInstr ret_instr = i->back();
                    i->body.pop_back();
                    if (alloca_instr) {
                        i->body.push_back(make_store_instr(alloca_instr.get(),
                                                ret_instr->operand(0)->usee));
                    }
                    i->body.push_back(make_br_instr(backBlock->label()));
                }
            }
            // Step 5: load ret value (if exists)
            if (alloca_instr) {
                auto load_instr = make_load_instr(alloca_instr.get());
                backBlock->push_after_label(load_instr);
                call_instr->replace_self(load_instr.get());
            }
            // Step 6: add new blocks to original function
            for (auto i : new_p.blocks) {
                fun->blocks.push_back(i);
            }
            fun->blocks.push_back(backBlock);
        }
    }
}

void FuncDefined::end_function() {
    if (body.empty() || body.back()->instr_type() != INSTR_RET) {
        if (ty->type_type() == TYPE_VOID_TYPE) {
            body.push_back(make_ret_instr());
        } else {
            body.push_back(make_unreachable_instr());
        }
    }
    Instrs final;
    final.push_back(Ir::make_label_instr());
    for (const auto &i : body) {
        if (i->instr_type() == INSTR_ALLOCA) {
            final.push_back(i);
        }
    }
    final.push_back(Ir::make_br_instr(body.front()));
    for (const auto &i : body) {
        if (i->instr_type() != INSTR_ALLOCA) {
            final.push_back(i);
        }
    }
    body.clear();
    p.from_instrs(final, args, imms);
}

String FuncDefined::print_func() const {
    String whole_function = "define " + ty->type_name() + " @" + name() +
                            "("; // functions are all global

    my_assert(!p.blocks.empty(), "Error: function has no block");

    auto func_ty = function_type();

    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            whole_function += ", ";
        }
        whole_function += func_ty->arg_type[i]->type_name();
        whole_function += " %";
        whole_function += arg_name[i];
    }

    whole_function += ")";

    whole_function += " {\n";
    for (const auto &i : p.blocks) {
        whole_function += i->print_block();
    }
    whole_function += "}\n";

    return whole_function;
}

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms) {
    return std::make_shared<FuncDefined>(std::move(var), std::move(arg_types),
                                         std::move(syms));
}

} // namespace Ir
