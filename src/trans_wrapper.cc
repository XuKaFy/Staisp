#include "trans_wrapper.h"
#include "def.h"
#include "ir_call_instr.h"
#include "ir_func.h"
#include "ir_func_defined.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_val.h"
#include "type.h"
#include <cstdio>

namespace Optimize {
bool memoi_wrapper::is_purely_recursive(Ir::FuncDefined *func) {

    if (func->function_type()->ret_type->type_type() == TYPE_VOID_TYPE ||
        func->function_type()->arg_type.empty())
        return false;
    for (auto &&arg_type : func->function_type()->arg_type) {
        if (!is_basic_type(arg_type))
            return false;
    }
    bool recursive = false;
    for (auto &&block : func->p) {
        for (auto &&instr : *block) {
            switch (instr->instr_type()) {
            case Ir::INSTR_CALL: {
                auto call = static_cast<Ir::CallInstr *>(instr.get());
                auto casted_func = call->operand(0)->usee;
                if (casted_func != func) { // recursion do not cancel purity
                    if (!is_purely_recursive(
                            dynamic_cast<Ir::FuncDefined *>(casted_func))) {
                        return false;
                    }
                } else {
                    recursive = true;
                }
                break;
            }
            case Ir::INSTR_LOAD: {
                auto src_ptr = instr->operand(0)->usee;
                if (src_ptr->type() == Ir::VAL_GLOBAL) {
                    auto src_global_val = static_cast<Ir::Global *>(src_ptr);
                    Vector<Ir::StoreInstr *> store_of_src;
                    for (auto &&src_uses : src_ptr->users()) {
                        if (auto src_usee =
                                dynamic_cast<Ir::StoreInstr *>(src_uses->user))
                            store_of_src.push_back(src_usee);
                    }
                    my_assert(
                        src_ptr->users().empty() ||
                            (!store_of_src.empty() || src_global_val->is_const),
                        "users != 0 -> store !=0");
                    if (store_of_src.size() == 1) {
                        break;
                    }
                }
            }
            case Ir::INSTR_ITEM:
            case Ir::INSTR_STORE: {
                if (instr->operand(0)->usee->name()[0] == '@') {
                    return false;
                }
                break;
            }
            }
        }
    }
    return recursive;
}

void memoi_wrapper::ap() {

    for (auto &&funcDef : module->funsDefined) {
        auto func = funcDef.get();
        if (!is_purely_recursive(func)) {
            return;
        }

        auto func_args = func->arg_name;
        auto func_ty = func->function_type();

        auto cur_cache =
            Ir::make_func(TypedSym(func->name() + "_cached", func_ty->ret_type),
                          func_ty->arg_type);
        for (auto &&func_uses : func->users()) {
            auto user = func_uses->user;
            auto call_instr_func = dynamic_cast<Ir::CallInstr *>(user);
            call_instr_func->change_operand(0, cur_cache.get());
            my_assert(call_instr_func, "must be call ");
        }
        module->add_func_declaration(cur_cache);
    }

    // printf("can be recursive: %s\n", func->name().c_str());
}

void memoi_wrapper::bk_fill(String &res) {}

} // namespace Optimize