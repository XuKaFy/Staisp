#pragma once

#include "def.h"
#include "ast_node.h"
#include "common_node.h"

namespace Ast {

struct ContinueException { };
struct BreakException { };
struct ReturnException {
    Immediate val;
};

Immediate must_have_value(ImmOrVoid imm, pNode root);

struct Executor {
    Executor(AstProg cur_prog);

    ImmOrVoid execute(pNode root);
    Immediate must_have_value_execute(pNode root);

private:
    ImmOrVoid execute_call(Pointer<OprNode> root);
    ImmOrVoid execute_func(Pointer<FuncDefNode> func, Immediates args);

    Pointer<FuncDefNode> find_const_function(Symbol sym, pNode root);

    EnvWrapper<Immediate> _env;
    AstProg _prog;
};


} // namespace ast
