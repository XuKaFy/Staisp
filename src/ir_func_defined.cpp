#include "ir_func_defined.h"

#include "imm.h"
#include "ir_call_instr.h"
#include "ir_constant.h"
#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "type.h"

namespace Ir {

FuncDefined::FuncDefined(TypedSym var, Vector<pType> arg_types,
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

void FuncDefined::add_body(pInstr instr) {
    body.push_back(instr);
}

void FuncDefined::add_imm(pVal val) { imms.push_back(val); }

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
    for(auto i : body) {
        if(i->instr_type() == INSTR_ALLOCA)
            final.push_back(i);
    }
    final.push_back(Ir::make_br_instr(body.front()));
    for(auto i : body) {
        if(i->instr_type() != INSTR_ALLOCA)
            final.push_back(i);
    }
    body.clear();
    p.from_instrs(final);
}

String FuncDefined::print_func() const {
    String whole_function = "define " + ty->type_name() + " @" + name() +
                            "("; // functions are all global

    my_assert(!p.blocks.empty(), "Error: function has no block");

    auto func_ty = functon_type();

    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0)
            whole_function += ", ";
        whole_function += func_ty->arg_type[i]->type_name();
        whole_function += " %";
        whole_function += arg_name[i];
    }

    whole_function += ")";

    whole_function += " {\n";
    for (auto i : p.blocks) {
        whole_function += i->print_block();
    }
    whole_function += "}\n";

    return whole_function;
}

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types,
                               Vector<String> syms) {
    return pFuncDefined(new FuncDefined(var, arg_types, syms));
}

} // namespace Ir
