#pragma once

#include <utility>

#include "ir_val.h"

namespace Ir {

struct Func : public User {
    Func(const TypedSym &var, Vector<pType> arg_types,
         bool variant_length = false)
        : User(make_function_type(var.ty, std::move(arg_types))),
          variant_length(variant_length) {
        set_name(var.sym);
    }

    ValType type() const override { return VAL_FUNC; }

    String print_func_declaration() const {
        String whole_function = "declare " + ty->type_name() + " @" + name() +
                                "("; // functions are all global
        auto func_ty = functon_type();

        // By Yaossg
        for (size_t i = 0; i < func_ty->arg_type.size(); ++i) {
            if (i > 0) {
                whole_function += ", ";
            }
            whole_function += func_ty->arg_type[i]->type_name();
        }

        if (variant_length) {
            whole_function += ", ...";
        }

        whole_function += ")";
        return whole_function;
    }

    bool variant_length;

    pFunctionType functon_type() const;
};

using pFunc = Pointer<Func>;

pFunc make_func(const TypedSym &var, Vector<pType> arg_types,
                bool variant_length = false);

} // namespace Ir
