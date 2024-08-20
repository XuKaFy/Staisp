#pragma once

#include <ir_constant.h>

#include "ir_block.h"
#include "ir_func.h"

#include "ast_node.h"

namespace Ir {

struct FunctionContext;

struct FuncDefined : public Func {
    FuncDefined(const TypedSym &var, Vector<pType> arg_types,
                Vector<String> arg_name);

    [[nodiscard]] String print_func() const;

    void end_function(FunctionContext& context);

    BlockedProgram p;

    Vector<String> arg_name;

    bool is_pure();
};

using pFuncDefined = Pointer<FuncDefined>;

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms);

struct FunctionContext {
    void clear();
    void init(Ir::pFuncDefined func);

    ConstPool cpool;
    Vector<pVal> params;
    Vector<pInstr> args;
    Ir::Instrs body;
    pFunctionType func_type;
};

} // namespace Ir
