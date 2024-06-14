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

void FuncDefined::end_function(Instrs &body, Vector<pVal> &params, Vector<pVal> &imms) {
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
    p.from_instrs(final, params, imms);
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
        whole_function += " %";
        whole_function += arg_name[i];
    }

    whole_function += ")";

    whole_function += " {\n";
    for (auto i = p.cbegin(); i != p.cend(); ++i) {
        whole_function += (*i)->print_block();
    }
    whole_function += "}\n";

    return whole_function;
}

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms) {
    return std::make_shared<FuncDefined>(std::move(var), std::move(arg_types),
                                         std::move(syms));
}

void FunctionContext::clear() {
    imms.clear();
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
        pInstr stored = make_store_instr(alloced, sym_node);
        body.push_back(alloced);
        body.push_back(stored);
        params.push_back(sym_node);
        args.push_back(alloced);
    }
}

} // namespace Ir
