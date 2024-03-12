#pragma once

#include "def.h"
#include "env.h"
#include "imm.h"
#include "value.h"

#include "ast_node.h"
#include "common_node.h"

namespace Ast {

struct ContinueException { };
struct BreakException { };
struct ReturnException {
    pValue val;
};

pValue must_have_value(LValueOrVoid imm, pNode root);

struct Executor {
    Executor(AstProg cur_prog);

    LValueOrVoid execute(pNode root);
    pValue must_have_value_execute(pNode root);

    static void throw_error(pNode root, int id, Symbol msg);

private:
    LValueOrVoid execute_call(Pointer<OprNode> root);
    LValueOrVoid execute_func(Pointer<FuncDefNode> func, Values args);

    pValue array_init(pType arr_type, pNode root);

    Pointer<FuncDefNode> find_const_function(Symbol sym, pNode root);

    EnvWrapper<pValue> _env;
    AstProg _prog;
};

} // namespace ast
