#pragma once

#include "def.h"
#include "env.h"
#include "imm.h"

#include "ast_node.h"
#include "common_node.h"

namespace Ast {

struct ContinueException { };
struct BreakException { };
struct ReturnException {
    ImmValue val;
};

ImmValue must_have_value(ImmOrVoid imm, pNode root);

struct Executor {
    Executor(AstProg cur_prog);

    ImmOrVoid execute(pNode root);
    ImmValue must_have_value_execute(pNode root);

    static void throw_error(pNode root, int id, Symbol msg);

private:
    ImmOrVoid execute_call(Pointer<OprNode> root);
    ImmOrVoid execute_func(Pointer<FuncDefNode> func, ImmValues args);

    Pointer<FuncDefNode> find_const_function(Symbol sym, pNode root);

    EnvWrapper<ImmValue> _env;
    AstProg _prog;
};

} // namespace ast
