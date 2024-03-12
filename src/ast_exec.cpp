#include "ast_exec.h"

namespace Ast {

void Executor::throw_error(pNode root, int id, Symbol msg)
{
    root->token->throw_error(id, "Executor", msg);
}

pValue must_have_value(LValueOrVoid imm, pNode root)
{
    if(imm.has_value()) {
        return imm.value();
    }
    Executor::throw_error(root, 7, "empty value");
    return pValue();
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

pValue Executor::must_have_value_execute(pNode root)
{
    return must_have_value(execute(root), root);
}

LValueOrVoid Executor::execute(pNode root)
{
    try {
        switch(root->type) {
        case NODE_IMM:
            return make_value(std::static_pointer_cast<ImmNode>(root)->imm);
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
            throw_error(root, 6, "function has no return");
        }
        case NODE_DEF_VAR: {
            auto r = std::static_pointer_cast<VarDefNode>(root);
            _env.env()->set(r->var.sym, must_have_value_execute(r->val));
            return LValueOrVoid();
        }
        case NODE_SYM: {
            auto sym = std::static_pointer_cast<SymNode>(root)->sym;
            if(_env.env()->count(sym))
                return _env.env()->find(sym);
            throw_error(root, 3, "variant not found");
        }
        case NODE_ASSIGN: {
            auto r = std::static_pointer_cast<AssignNode>(root);
            pValue lv = must_have_value_execute(r->lv);
            pValue rv = must_have_value_execute(r->val);
            lv->reset_value(*rv);
            return LValueOrVoid();
        }
        case NODE_ARRAY_VAL: { // impossible
            auto r = std::static_pointer_cast<ArrayDefNode>(root);
            Values vals;
            for(auto i : r->nums) {
                vals.push_back(must_have_value_execute(i));
            }
            return LValueOrVoid();
        }
        case NODE_ITEM: {
            auto r = std::static_pointer_cast<ItemNode>(root);
            const pValue &v = _env.env()->find(r->v);
            for(auto i : r->index) {
                pValue v = must_have_value_execute(i);
                if(v->type() != VALUE_IMM || !is_imm_integer(v->imm_value().ty)) {
                    throw_error(i, 11, "type of index should be integer");
                }
                ImmValue index = v->imm_value();
                if(v->type() != VALUE_ARRAY) {
                    throw_error(i, 12, "index used to non-array type");
                }
                if(is_imm_signed(index.ty)) {
                    long long ii = index.val.ival;
                    v = v->array_value()[ii];
                } else {
                    unsigned long long ui = index.val.uval;
                    v = v->array_value()[ui];
                }
            }
            return v;
        }
        case NODE_CAST: {
            auto r = std::static_pointer_cast<CastNode>(root);
            pValue casted = must_have_value_execute(r->val);
            if(!is_basic_type(r->ty) || casted->type() != VALUE_IMM)
                throw_error(r, 13, "bad cast");
            return make_value(casted->imm_value().cast_to(to_basic_type(r->ty)->ty));
        }
        case NODE_REF: {
            auto r = std::static_pointer_cast<RefNode>(root);
            pValue v = _env.env()->find(r->name);
            PointerValue pv {v};
            return make_value(pv);
        }
        case NODE_DEREF: {
            auto r = std::static_pointer_cast<DerefNode>(root);
            pValue v = must_have_value_execute(r->val);
            if(v->type() != VALUE_POINTER)
                throw_error(root, 9, "dereference of non-pointer type");
            return v->pointer_value().v;
        }
        case NODE_DEF_CONST_FUNC:
        case NODE_DEF_FUNC:
            throw_error(root, 5, "function nested");
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
            if(a1->type() == VALUE_IMM && a2->type() == VALUE_IMM) \
                return make_value(a1->imm_value() y a2->imm_value()); \
            throw_error(root, 8, "operation of non-immediate value"); \
        }
        OPR_TABLE_CALCULATABLE
    #undef ENTRY
        case OPR_IF: {
            if(r->ch.size() == 2) {
                if(*must_have_value_execute(r->ch.front()))
                    execute(r->ch.back());
            } else {
                if(*must_have_value_execute(r->ch.front())) {
                    auto j = r->ch.begin();
                    ++j;
                    execute(*j);
                }
                else
                    execute(r->ch.back());
            }
            return LValueOrVoid();
        }
        case OPR_WHILE: {
            while(*must_have_value_execute(r->ch.front())) {
                try {
                    execute(r->ch.back());
                } catch (ContinueException) {
                    continue;
                } catch (BreakException) {
                    break;
                }
            }
            return LValueOrVoid();
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
        return LValueOrVoid();
    } catch (Exception e) {
        root->throw_error(e.id, e.object, e.message);
    }
    return LValueOrVoid();
}

LValueOrVoid Executor::execute_call(Pointer<OprNode> root)
{
    Symbol name = nullptr;
    auto args = Values();
    for(auto i : root->ch) {
        if(!name) {
            name = std::static_pointer_cast<SymNode>(i)->sym;
        } else {
            args.push_back(must_have_value_execute(i));
        }
    }
    return execute_func(find_const_function(name, root), args);
}

LValueOrVoid Executor::execute_func(Pointer<FuncDefNode> func, Values args)
{
    if(func->args.size() != args.size())
        throw_error(func, 4, "wrong count of arguments");
    _env.push_env();
    for(size_t i=0; i<args.size(); ++i) {
        _env.env()->set(func->args[i].sym, args[i]);
    }
    LValueOrVoid ans;
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
            throw_error(i, 1, "function is not declared as DEFCONSTFUNC");
        }
    }
    throw_error(root, 2, "function not found");
    return Pointer<FuncDefNode>();
}

} // namespace ast
