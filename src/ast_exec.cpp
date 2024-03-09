#include "ast_exec.h"

namespace Ast {

ImmValue must_have_value(ImmOrVoid imm, pNode root)
{
    if(imm.has_value())
        return imm.value();
    root->token->print_error("[Executor] error 7: empty value");
    return -1;
}

Executor::Executor(AstProg cur_prog)
    : _prog(cur_prog)
{
    _env.push_env();
    for(auto i : cur_prog) {
        if(i->type == NODE_DEF_VAR) {
            auto r = std::static_pointer_cast<VarDefNode>(i);
            _env.env()->set(r->var.sym, must_have_value_execute(r->val));
        }
    }
}

ImmValue Executor::must_have_value_execute(pNode root)
{
    return must_have_value(execute(root), root);
}

ImmOrVoid Executor::execute(pNode root)
{
    switch(root->type) {
    case NODE_IMM:
        return std::static_pointer_cast<ImmNode>(root)->imm;
    case NODE_BLOCK: {
        _env.push_env();
        try {
            for(auto i : std::static_pointer_cast<BlockNode>(root)->body)
                execute(i);
        } catch (ReturnException e) { // [!!!!!!!!!!!!ATTENTION!!!!!!!!!!!!]
            _env.end_env();
            throw e;
        }
        _env.end_env();
        root->token->print_error("[Executor] error 6: function has no return");
    }
    case NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<VarDefNode>(root);
        _env.env()->set(r->var.sym, must_have_value_execute(r->val));
        return ImmOrVoid();
    }
    case NODE_SYM: {
        auto sym = std::static_pointer_cast<SymNode>(root)->sym;
        if(_env.env()->count(sym))
            return _env.env()->find(sym);
        root->token->print_error("[Executor] error 3: variant not found");
    }
    case NODE_ASSIGN: {
        auto r = std::static_pointer_cast<AssignNode>(root);
        if(r->lv->type != NODE_SYM) {
            my_assert(false, "not implemented");
        }
        auto lv = std::static_pointer_cast<SymNode>(r->lv);
        _env.env()->set(lv->sym, must_have_value_execute(r->val));
    }
    case NODE_DEF_CONST_FUNC:
    case NODE_DEF_FUNC:
        root->token->print_error("[Executor] error 5: function nested");
    default: break;
    }
    // NODE_OPR
    auto r = std::static_pointer_cast<OprNode>(root);
    switch(r->type) {
#define ENTRY(x, y) \
    case OPR_##x: { \
        my_assert(r->ch.size() == 2, "Error: constexpr operator has wrong count of args."); \
        auto a1 = must_have_value_execute(r->ch.front()); \
        auto a2 = must_have_value_execute(r->ch.back()); \
        return a1 y a2; \
    }
    OPR_TABLE_CALCULATABLE
#undef ENTRY
    case OPR_IF: {
        if(r->ch.size() == 2) {
            if(must_have_value_execute(r->ch.front()))
                execute(r->ch.back());
        } else {
            if(must_have_value_execute(r->ch.front())) {
                auto j = r->ch.begin();
                ++j;
                execute(*j);
            }
            else
                execute(r->ch.back());
        }
        return ImmOrVoid();
    }
    case OPR_WHILE: {
        while(must_have_value_execute(r->ch.front())) {
            try {
                execute(r->ch.back());
            } catch (ContinueException) {
                continue;
            } catch (BreakException) {
                break;
            }
        }
        return ImmOrVoid();
    }
    case OPR_BREAK: {
        throw BreakException {};
    }
    case OPR_CONTINUE: {
        throw ContinueException {};
    }
    case OPR_CALL: {
        return execute_call(r);
    }
    case OPR_RET: {
        throw ReturnException { must_have_value_execute(r->ch.front()) };
    }
    }
    my_assert(false, "?");
    return ImmOrVoid();
}

ImmOrVoid Executor::execute_call(Pointer<OprNode> root)
{
    Symbol name = nullptr;
    auto args = ImmValues();
    for(auto i : root->ch) {
        if(!name) {
            name = std::static_pointer_cast<SymNode>(i)->sym;
        } else {
            args.push_back(must_have_value_execute(i));
        }
    }
    return execute_func(find_const_function(name, root), args);
}

ImmOrVoid Executor::execute_func(Pointer<FuncDefNode> func, ImmValues args)
{
    node_assert(func->args.size() == args.size(), func, "[Executor] error 4: wrong count of arguments");
    _env.push_env();
    for(size_t i=0; i<args.size(); ++i) {
        _env.env()->set(func->args[i].sym, args[i]);
    }
    ImmOrVoid ans;
    try {
        ans = execute(func->body);
    } catch(ReturnException e) {
        ans = e.val;
    }
    _env.end_env();
    return ans;
}

Pointer<FuncDefNode> Executor::find_const_function(Symbol sym, pNode root)
{
    for(auto i : _prog) {
        if(i->type == NODE_DEF_CONST_FUNC) {
            auto cname = std::static_pointer_cast<FuncDefNode>(i)->var.sym;
            if(String(sym) == String(cname)) {
                return std::static_pointer_cast<FuncDefNode>(i);
            }
            i->token->print_error("[Executor] error 1: function is not declared as DEFCONSTFUNC");
        }
    }
    root->token->print_error("[Executor] error 2: function not found");
    return Pointer<FuncDefNode>();
}

} // namespace ast
