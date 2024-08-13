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
    : Func(var, arg_types), arg_name(arg_name) {
}

void FuncDefined::end_function(FunctionContext& context) {
    Instrs &body = context.body;
    Vector<pVal> &params = context.params;
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
    final.push_back(Ir::make_br_instr(body.front().get()));
    for (const auto &i : body) {
        if (i->instr_type() != INSTR_ALLOCA) {
            final.push_back(i);
        }
    }
    body.clear();
    p.initialize(name(), std::move(final), std::move(params), std::move(context.cpool));
}

String FuncDefined::print_func() const {
    String whole_function = "define " + ty->type_name() + " @" + name() +
                            "("; // functions are all global

    my_assert(!p.empty(), "Error: function has no block");

    auto func_ty = function_type();

    for (size_t i = 0; i < func_ty->arg_type.size(); ++i) {
        if (i > 0) {
            whole_function += ", ";
        }
        whole_function += func_ty->arg_type[i]->type_name();
        whole_function += " ";
        whole_function += p.params()[i]->name();
    }

    whole_function += ")";

    whole_function += " {\n";
    for (auto i = p.cbegin(); i != p.cend(); ++i) {
        whole_function += (*i)->print_block();
    }
    whole_function += "}\n";

    return whole_function;
}


bool FuncDefined::is_pure() {
    // if (function_type()->ret_type->type_type() == TYPE_VOID_TYPE || function_type()->arg_type.empty()) return false;
    for (auto&& arg_type : function_type()->arg_type) {
        if (!is_basic_type(arg_type)) return false;
    }
    for (auto&& block : p) {
        for (auto&& instr : *block) {
            switch (instr->instr_type()) {
                case INSTR_CALL: {
                    auto call = static_cast<CallInstr*>(instr.get());
                    auto func = call->operand(0)->usee;
                    if (func != this) { // recursion do not cancel purity
                        auto callee = dynamic_cast<FuncDefined*>(func);
                        if (callee && callee->is_pure()) {
                            break;
                        }
                        return false;
                    }
                    break;
                }
                case INSTR_LOAD:
                case INSTR_ITEM:
                case INSTR_STORE:
                case INSTR_CAST:
                case INSTR_MINI_GEP: {
                    if (instr->operand(0)->usee->name()[0] == '@') {
                        return false;
                    }
                    break;
                }
            }
        }
    }
    return true;
}

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms) {
    return std::make_shared<FuncDefined>(std::move(var), std::move(arg_types),
                                         std::move(syms));
}

void FunctionContext::clear() {
    cpool.clear();
    body.clear();
    args.clear();
    params.clear();
    func_type = nullptr;
}

void FunctionContext::init(Ir::pFuncDefined func) {
    clear();

    func_type = func->function_type();

    Vector<pType> arg_types = func->function_type()->arg_type;
    size_t length = func_type->arg_type.size();
    body.push_back(make_label_instr());
    for (size_t i = 0; i < length; ++i) {
        auto sym_node =
            make_sym_instr(TypedSym("%" + func->arg_name[i], arg_types[i]));
        pInstr alloced = make_alloc_instr(arg_types[i]);
        pInstr stored = make_store_instr(alloced.get(), sym_node.get());
        body.push_back(alloced);
        body.push_back(stored);
        params.push_back(sym_node);
        args.push_back(alloced);
    }
}

} // namespace Ir
