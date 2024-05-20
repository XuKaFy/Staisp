#pragma once

#include "ir_val.h"

namespace Ir {

struct Func : public User {
    Func(TypedSym var, Vector<pType> arg_types, bool variant_length = false)
        : User(make_function_type(var.ty, arg_types)),
          variant_length(variant_length) {
        set_name(var.sym);
    }

    virtual ValType type() const { return VAL_FUNC; }

    Symbol print_func_declaration() const {
        String whole_function = String("declare ") + ty->type_name() + " @" +
                                name() + "("; // functions are all global
        auto func_ty = functon_type();

        if (func_ty->arg_type.empty())
            goto PRINT_IMPL_END;

        whole_function += func_ty->arg_type[0]->type_name();

        for (size_t i = 1; i < func_ty->arg_type.size(); ++i) {
            whole_function += ", ";
            whole_function += func_ty->arg_type[i]->type_name();
        }

        if (variant_length) {
            whole_function += ", ...";
        }

    PRINT_IMPL_END:
        whole_function += ")";
        return to_symbol(whole_function);
    }

    bool variant_length;

    pFunctionType functon_type() const;
};

typedef Pointer<Func> pFunc;

pFunc make_func(TypedSym var, Vector<pType> arg_types,
                bool variant_length = false);

} // namespace Ir
