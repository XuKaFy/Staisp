#pragma once

#include "ir_block.h"
#include "ir_func.h"

#include "ast_node.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(const TypedSym &var, Vector<pType> arg_types,
                Vector<String> arg_name);

    String print_func() const;

    void end_function(Instrs &body, Vector<pVal> &params, Vector<pVal> &imms);

    BlockedProgram p;
    Pointer<Ast::FuncDefNode> ast_root;

    Vector<String> arg_name;
};

using pFuncDefined = Pointer<FuncDefined>;

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms);

struct FunctionContext {
    void clear();
    void init(Ir::pFuncDefined func);

    Vector<pVal> imms;
    Vector<pVal> params;
    Vector<pInstr> args;
    Ir::Instrs body;
    pFunctionType func_type;
};

} // namespace Ir
