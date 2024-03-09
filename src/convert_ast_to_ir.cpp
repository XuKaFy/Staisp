#include "convert_ast_to_ir.h"

namespace AstToIr {

Ir::pModule Convertor::module() const
{
    return _mod;
}

Ir::pInstr Convertor::find_left_value(pNode root, Symbol sym, Ir::pFuncDefined func)
{
    if(_env.env()->count(sym)) { // local
        Ir::pInstr found = _env.env()->find(sym);
        // const value is error
        if(found->instrType != Ir::INSTR_TYPE_ALLOC) 
            node_assert(false, root, "[Convertor] error 10: assignment to a local const value");
        // no-const value should be loaded
        return found;
    } else { // global
        for(auto i : module()->globs) {
            if(strcmp(i->val.sym, sym) == 0) {
                if(i->val.tr->is_const) {
                    node_assert(false, root, "[Convertor] error 11: assignment to a global const value");
                }
                return Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
            }
        }
    }
    node_assert(false, root, "[Convertor] error 1: left value cannot be found");
    return Ir::make_empty_instr();
}

Ir::pInstr Convertor::find_left_value(pNode root, pNode lv, Ir::pFuncDefined func)
{
    if(lv->type == NODE_DEREF) {
        auto r = std::static_pointer_cast<Ast::DerefNode>(lv);
        auto res = analyze_value(r->val, func);
        return res;
    }
    if(lv->type == NODE_ITEM) {
        auto r = std::static_pointer_cast<Ast::ItemNode>(lv);
        node_assert(false, lv, "[Convertor] not implemented");
    }
    if(lv->type != NODE_SYM) {
        node_assert(false, lv, "[Convertor] error 12: expected to be a left-value");
    }
    Symbol sym = std::static_pointer_cast<Ast::SymNode>(lv)->sym;
    return find_left_value(root, sym, func);
}

Ir::pInstr Convertor::find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func)
{
    if(_env.env()->count(root->sym)) { // local
        Ir::pInstr found = _env.env()->find(root->sym);
        // no-const value should be loaded
        if(is_pointer(found->tr))
            return func->add_instr(Ir::make_load_instr(found));
        // const value should be immediately used
        return found;
    } else { // global
        for(auto i : module()->globs) {
            if(strcmp(i->val.sym, root->sym) == 0) {
                auto sym = Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
                return func->add_instr(Ir::make_load_instr(sym));
            }
        }
    }
    // impossible
    node_assert(false, root, "[Convertor] symbol cannot be found - impossible");
    return Ir::make_empty_instr();
}

Ir::pInstr Convertor::cast_to_type(pNode root, Ir::pInstr val, pType tr, Ir::pFuncDefined func)
{
    if(same_type(val->tr, tr)) return val;
    if(!castable(val->tr, tr)) {
        printf("Message: not castable from %s to %s\n", val->tr->type_name(), tr->type_name());
        root->token->print_error("[Convertor] error 13: not castable");
    }
    auto r = Ir::make_cast_instr(tr, val);
    func->add_instr(r);
    return r;
}

Ir::pInstr Convertor::analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func)
{
    switch(root->type) {
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MUL:
    case OPR_DIV:
    case OPR_AND:
    case OPR_OR:
    case OPR_REM: {
        // impossible
        node_assert(root->ch.size() == 2, root, "[Convertor] binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto an1 = *a;
        auto a1 = analyze_value(*(a++), func);
        auto an2 = *a;
        auto a2 = analyze_value(*(a++), func);
        auto joined_type = join_type(a1->tr, a2->tr);
        if(!joined_type)
            root->token->print_error("[Type] error 1: type has no joined type");
        auto ir = Ir::make_binary_instr(fromBinaryOpr(root), joined_type, cast_to_type(an1, a1, joined_type, func), cast_to_type(an2, a2, joined_type, func));
        func->add_instr(ir);
        return ir;
    }
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE: {
        // impossible
        node_assert(root->ch.size() == 2, root, "[Convertor] binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto an1 = *a;
        auto a1 = analyze_value(*(a++), func);
        auto an2 = *a;
        auto a2 = analyze_value(*(a++), func);
        auto ty = join_type(a1->tr, a2->tr);
        if(!ty)
            root->token->print_error("[Type] error 1: type has no joined type");
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root, ty), ty, cast_to_type(an1, a1, ty, func), cast_to_type(an2, a2, ty, func));
        func->add_instr(ir);
        return ir;
    }
    case OPR_IF: {
        auto trueBlock = Ir::make_block();
        auto falseBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();
        // impossible
        node_assert(root->ch.size() == 3 || root->ch.size() == 2, root, "[Convertor] \"IF\" has not 2 or 3 args - impossible");
        
        auto a = root->ch.begin();
        auto cond = analyze_value(*(a++), func);
        // printf("for if cond, tr = %d\n", cond->tr);
        if(root->ch.size() == 3) {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), falseBlock->label()));
        } else {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));
        }

        func->add_block(trueBlock);
        analyze_statement_node(*(a++), func);
        trueBlock->finish_block_with_jump(afterBlock);
        
        if(root->ch.size() == 3) {
            func->add_block(falseBlock);
            analyze_statement_node(*(a++), func);
            falseBlock->finish_block_with_jump(afterBlock);
        }
        
        func->add_block(afterBlock);
        return Ir::make_empty_instr();
    }
    case OPR_WHILE: {
        auto compBlock = Ir::make_block();
        auto trueBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();

        // impossible
        node_assert(root->ch.size() == 3 || root->ch.size() == 2, root, "[Convertor] \"WHILE\" opr has not 2 args - impossible");

        push_loop_env(pLoopEnv(new LoopEnv { compBlock, afterBlock } ));

        auto a = root->ch.begin();

        func->current_block()->finish_block_with_jump(compBlock);
        
        func->add_block(compBlock);
        auto cond = analyze_value(*(a++), func);
        compBlock->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));

        func->add_block(trueBlock);
        analyze_statement_node(*(a++), func);   
        func->current_block()->finish_block_with_jump(compBlock); // may have other block caused by break or continue
        
        func->add_block(afterBlock);

        end_loop_env();

        return Ir::make_empty_instr();
    }
    case OPR_RET: {
        // impossible
        node_assert(root->ch.size() == 1, root, "[Convertor] \"RET\" opr has not 1 args - impossible");
        auto a = analyze_value(root->ch.front(), func);
        func->add_instr(Ir::make_ret_instr(func->var.tr, cast_to_type(root->ch.front(), a, func->var.tr, func)));
        return Ir::make_empty_instr();
    }
    case OPR_CALL: {
        // impossible
        node_assert(root->ch.size() >= 1, root, "[Convertor] \"CALL\" opr has less than 1 args - impossible");
        Vector<Ir::pInstr> args;
        // impossible
        node_assert(root->ch.front()->type == NODE_SYM, root, "[Convertor] \"CALL\" opr gets a name that not sym - impossible");
        for(auto i=++root->ch.begin(); i!=root->ch.end(); ++i) {
            args.push_back(analyze_value(*i, func));
        }
        auto name_node = std::static_pointer_cast<Ast::SymNode>(root->ch.front());
        auto func_instr = find_func(name_node->sym);
        node_assert(func_instr->args.size() == root->ch.size() - 1, root, "[Convertor] error 8: wrong count of arguments");
        auto func_ir = Ir::make_call_instr(func_instr, args);
        func->add_instr(func_ir);
        return func_ir;
    }
    case OPR_BREAK: {
        // impossible
        node_assert(root->ch.size() == 0, root, "[Convertor] \"BREAK\" opr has less than 1 args - impossible");
        node_assert(has_loop_env(), root, "[Convertor] error 9: no outer loops");
        
        auto r = Ir::make_br_instr(loop_env()->loop_end->label());
        func->add_instr(r);
        // auto nextBlock = Ir::make_block();
        // func->add_block(nextBlock);
        return Ir::make_empty_instr();
    }
    case OPR_CONTINUE: {
        // impossible
        node_assert(root->ch.size() == 0, root, "[Convertor] \"CONTINUE\" opr has less than 1 args - impossible");
        node_assert(has_loop_env(), root, "[Convertor] error 9: no outer loops");
        
        auto r = Ir::make_br_instr(loop_env()->loop_begin->label());
        func->add_instr(r);
        // auto nextBlock = Ir::make_block();
        // func->add_block(nextBlock);
        return Ir::make_empty_instr();
    }
    default:
        node_assert(false, root, "[Convertor] error 2: operation not implemented");
        return Ir::make_empty_instr();
    }
}

Ir::pInstr Convertor::analyze_value(pNode root, Ir::pFuncDefined func)
{
    switch(root->type) {
    case NODE_CAST: {
        auto r = std::static_pointer_cast<Ast::CastNode>(root);
        auto res = Ir::make_cast_instr(r->ty, analyze_value(r->val, func));
        func->add_instr(res);
        return res;
    }
    case NODE_OPR: {
        auto ir = analyze_opr(std::static_pointer_cast<Ast::OprNode>(root), func);
        return ir;
    }
    case NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_binary_instr(Ir::INSTR_ADD, make_basic_type(r->imm.ty, false),
                     Ir::make_constant(r->imm.ty), Ir::make_constant(r->imm));
        func->add_instr(res);
        return res;
    }
    case NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        auto to = find_value(r, func);
        return to;
    }
    case NODE_REF: {
        auto r = std::static_pointer_cast<Ast::RefNode>(root);
        return find_left_value(root, r->name, func);
    }
    case NODE_DEREF: {
        auto r = std::static_pointer_cast<Ast::DerefNode>(root);
        auto res = Ir::make_load_instr(analyze_value(r->val, func));
        func->add_instr(res);
        return res;
    }
    case NODE_ITEM: {
        auto r = std::static_pointer_cast<Ast::ItemNode>(root);
        auto sym = r->v;
        auto index = analyze_value(r->index, func);
        node_assert(is_integer(index->tr), r->index, "[Convertor] error 14: type of index should be integer");
        return func->add_instr(Ir::make_item_instr(find_left_value(r, sym, func), index));
    }
    case NODE_ARRAY_VAL: {
        node_assert(false, root, "[Convertor] array definition is not calculatable - impossible");
    }
    case NODE_ASSIGN:
    case NODE_DEF_VAR:
    case NODE_DEF_FUNC:
    case NODE_BLOCK:
    default:
        node_assert(false, root, "[Convertor] error 3: node not calculatable");
        return Ir::make_empty_instr();
    }
}

void Convertor::copy_to_array(TypedSym var, Vector<pNode> nums, Ir::pFuncDefined func)
{
    // TODO TODO TODO
    printf("ready to define %s\n", var.sym);
}

void Convertor::analyze_statement_node(pNode root, Ir::pFuncDefined func)
{
    switch(root->type) {
    case NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_left_value(r, r->lv, func);
        auto j = to_pointed_type(to->tr);
        func->add_instr(Ir::make_store_instr(to, cast_to_type(r->val, analyze_value(r->val, func), j, func)));
        break;
    }
    case NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        if(r->var.tr->type_type() == TYPE_COMPOUND_TYPE) {
            auto rr = std::static_pointer_cast<CompoundType>(r->var.tr)->compound_type_type();
            if(rr == COMPOUND_TYPE_ARRAY) {
                node_assert(r->val->type == NODE_ARRAY_VAL, r->val, "[Convertor] error 15: array should be initialized by a list");
                auto rrr = std::static_pointer_cast<Ast::ArrayDefNode>(r->val);
                copy_to_array(r->var, rrr->nums, func);
                break;
            }
        }
        if(r->var.tr->is_const) {
            func->add_instr(tmp = Ir::make_binary_instr(Ir::INSTR_ADD, r->var.tr, 
                Ir::make_constant(r->var.tr), cast_to_type(r->val, analyze_value(r->val, func), r->var.tr, func)));
            _env.env()->set(r->var.sym, tmp);
        } else {
            func->add_instr(tmp = Ir::make_alloc_instr(r->var.tr));
            func->add_instr(Ir::make_store_instr(tmp, cast_to_type(r->val, analyze_value(r->val, func), r->var.tr, func)));
            _env.env()->set(r->var.sym, tmp);
        }
        break;
    }
    case NODE_OPR:
        analyze_value(root, func);
        break;
    case NODE_BLOCK: {
        auto r = std::static_pointer_cast<Ast::BlockNode>(root);
        _env.push_env();
        for(auto i : r->body) {
            analyze_statement_node(i, func);
        }
        _env.end_env();
        break;
    }
    case NODE_ITEM:
    case NODE_CAST:
    case NODE_IMM:
    case NODE_SYM:
    case NODE_REF:
    case NODE_DEREF:
    case NODE_ARRAY_VAL:
        break; // meaningless
    case NODE_DEF_CONST_FUNC:
    case NODE_DEF_FUNC:
        // impossible
        node_assert(false, root, "[Convertor] function nested - impossible");
    }
}

void Convertor::generate_function(Pointer<Ast::FuncDefNode> root)
{
    _env.push_env();
    Ir::pFuncDefined func;
    {
        func = Ir::make_func_defined(root->var, root->args);
        set_func(func->var.sym, func);
        my_assert(root->args.size() == func->args_value.size(), "Error: inner error.");
        for(size_t i=0; i<root->args.size(); ++i) {
            _env.env()->set(root->args[i].sym, func->args_value[i]);
        }
    }
    analyze_statement_node(root->body, func);
    module()->add_func(func);
    _env.end_env();
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root)
{
    _env.push_env();
    module()->add_global(Ir::make_global(root->var, Ast::Executor(_prog).must_have_value_execute(root->val)));
    _env.end_env();
}

void Convertor::generate_single(pNode root)
{
    switch(root->type) {
    case NODE_DEF_CONST_FUNC:
    case NODE_DEF_FUNC:
        generate_function(std::static_pointer_cast<Ast::FuncDefNode>(root));
        break;
    case NODE_DEF_VAR:
        generate_global_var(std::static_pointer_cast<Ast::VarDefNode>(root));
        break;
    default:
        node_assert(false, root, "[Convertor] error 5: global operation has type that not implemented");
    }
}

Ir::pModule Convertor::generate(AstProg asts)
{
    _env.clear_env();
    clear_loop_env();
    _prog = asts;
    _mod = Ir::pModule(new Ir::Module());
    for(auto i : asts) {
        generate_single(i);
    }
    return _mod;
}

Ir::BinInstrType Convertor::fromBinaryOpr(Pointer<Ast::OprNode> root)
{
#define SELECT(x) case OPR_##x: return Ir::INSTR_##x;
    switch(root->type) {
    SELECT(ADD)
    SELECT(SUB)
    SELECT(MUL)
    SELECT(DIV)
    SELECT(REM)
    SELECT(AND)
    SELECT(OR)
    default:
        node_assert(false, root, "[Convertor] error 6: binary operation conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::INSTR_ADD;
}

Ir::CmpType Convertor::fromCmpOpr(Pointer<Ast::OprNode> root, pType tr)
{
    bool is_signed = is_signed_type(tr);
#define SELECT(x) case OPR_##x: return Ir::CMP_##x;
    switch(root->type) {
    SELECT(EQ)
    SELECT(NE)
#undef SELECT
#define SELECT_US(x) \
    case OPR_##x: { \
        if(is_signed) { \
            return Ir::CMP_S##x; \
        } else { \
            return Ir::CMP_U##x; \
        } \
        break; \
    }
    SELECT_US(LE)
    SELECT_US(GE)
    SELECT_US(LT)
    SELECT_US(GT)
#undef SELECT
    default:
        node_assert(false, std::static_pointer_cast<Node>(root), "[Convertor] error 7: comparasion operation conversion from ast to ir not implemented");
    }
    return Ir::CMP_EQ;
}

pLoopEnv Convertor::loop_env() {
    if(_loop_env_stack.empty())
        return _loop_env_stack.top();
    return _loop_env_stack.top();
}

void Convertor::push_loop_env(pLoopEnv env) {
    _loop_env_stack.push(env);
}

bool Convertor::has_loop_env() const {
    return !_loop_env_stack.empty();
}
    
void Convertor::end_loop_env() {
    _loop_env_stack.pop();
}
    
void Convertor::clear_loop_env() {
    while(!_loop_env_stack.empty())
        _loop_env_stack.pop();
}

void Convertor::set_func(Symbol sym, Ir::pFunc fun) {
    _func_map[sym] = fun;
}

bool Convertor::func_count(Symbol sym) {
    return _func_map.count(sym);
}

Ir::pFunc Convertor::find_func(Symbol sym) {
    return _func_map[sym];
}

} // namespace ast_to_ir

