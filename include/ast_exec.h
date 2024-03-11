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
    Value val;
};

Value must_have_value(ValueOrVoid imm, pNode root);

struct Executor {
    Executor(AstProg cur_prog);

    ValueOrVoid execute(pNode root);
    Value must_have_value_execute(pNode root);

    static void throw_error(pNode root, int id, Symbol msg);

private:
    ValueOrVoid execute_call(Pointer<OprNode> root);
    ValueOrVoid execute_func(Pointer<FuncDefNode> func, Values args);

    Pointer<FuncDefNode> find_const_function(Symbol sym, pNode root);

    EnvWrapper<Value> _env;
    AstProg _prog;
};

} // namespace ast
