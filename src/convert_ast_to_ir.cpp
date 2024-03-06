#include "convert_ast_to_ir.h"

namespace AstToIr {

bool Env::var_count(Symbol sym) {
    if(_var_map.count(sym)) return true;
    if(_parent) return _parent->var_count(sym);
    return false;
}

Ir::pInstr Env::find_var(Symbol sym) {
    if(_var_map.count(sym))
        return _var_map[sym];
    return _parent->find_var(sym);
}

void Env::set_var(Symbol sym, Ir::pInstr i) {
    _var_map[sym] = i;
}

void Convertor::node_assert(bool judge, pNode root, Symbol message)
{
    if(!judge) {
        root->token->print_error(message);
    }
}

Ir::pInstr Convertor::find_left_value(Pointer<Ast::AssignNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    if(env()->var_count(root->sym)) { // local
        return env()->find_var(root->sym);
    } else { // global
        for(auto i : mod->globs) {
            if(strcmp(i->val.sym, root->sym) == 0) {
                return Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
            }
        }
    }
    node_assert(false, root, "[Convertor] error 1: left value cannot be found");
    return Ir::make_empty_instr();
}

Ir::pInstr Convertor::find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    if(env()->var_count(root->sym)) { // local
        return func->add_instr(Ir::make_load_instr(env()->find_var(root->sym)->tr, env()->find_var(root->sym)));
    } else { // global
        for(auto i : mod->globs) {
            if(strcmp(i->val.sym, root->sym) == 0) {
                auto sym = Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
                return func->add_instr(Ir::make_load_instr(sym->tr, sym));
            }
        }
    }
    // impossible
    node_assert(false, root, "[Convertor] symbol cannot be found - impossible");
    return Ir::make_empty_instr();
}

Ir::pInstr Convertor::analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MUL:
    case OPR_SDIV:
    case OPR_AND:
    case OPR_OR:
    case OPR_REM: {
        // impossible
        node_assert(root->ch.size() == 2, root, "[Convertor] binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto a1 = analyze_value(*(a++), func, mod);
        auto a2 = analyze_value(*(a++), func, mod);
        // printf("[exec %d, joined_type = %d]\n", root->type, join_type(a1->tr, a2->tr));
        auto ir = Ir::make_binary_instr(fromBinaryOpr(root), join_type(a1->tr, a2->tr), a1, a2);
        func->add_instr(ir);
        return ir;
    }
    case OPR_EQ:
    case OPR_NE:
    case OPR_UGT:
    case OPR_UGE:
    case OPR_ULT:
    case OPR_ULE:
    case OPR_SGT:
    case OPR_SGE:
    case OPR_SLT:
    case OPR_SLE: {
        // impossible
        node_assert(root->ch.size() == 2, root, "[Convertor] binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto a1 = analyze_value(*(a++), func, mod);
        auto a2 = analyze_value(*(a++), func, mod);
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root), join_type(a1->tr, a2->tr), a1, a2);
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
        auto cond = analyze_value(*(a++), func, mod);
        // printf("for if cond, tr = %d\n", cond->tr);
        if(root->ch.size() == 3) {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), falseBlock->label()));
        } else {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));
        }

        func->add_block(trueBlock);
        analyze_statement_node(*(a++), func, mod);
        trueBlock->finish_block_with_jump(afterBlock);
        
        if(root->ch.size() == 3) {
            func->add_block(falseBlock);
            analyze_statement_node(*(a++), func, mod);
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
        auto cond = analyze_value(*(a++), func, mod);
        compBlock->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));

        func->add_block(trueBlock);
        analyze_statement_node(*(a++), func, mod);   
        func->current_block()->finish_block_with_jump(compBlock); // may have other block caused by break or continue
        
        func->add_block(afterBlock);

        end_loop_env();

        return Ir::make_empty_instr();
    }
    case OPR_RET: {
        // impossible
        node_assert(root->ch.size() == 1, root, "[Convertor] \"RET\" opr has not 1 args - impossible");
        auto a = analyze_value(root->ch.front(), func, mod);
        func->add_instr(Ir::make_ret_instr(a->tr, a));
        return Ir::make_empty_instr();
    }
    case OPR_CALL: {
        // impossible
        node_assert(root->ch.size() >= 1, root, "[Convertor] \"CALL\" opr has less than 1 args - impossible");
        Vector<Ir::pInstr> args;
        // impossible
        node_assert(root->ch.front()->type == NODE_SYM, root, "[Convertor] \"CALL\" opr gets a name that not sym - impossible");
        for(auto i=++root->ch.begin(); i!=root->ch.end(); ++i) {
            args.push_back(analyze_value(*i, func, mod));
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

Ir::pInstr Convertor::analyze_value(pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case NODE_OPR: {
        auto ir = analyze_opr(std::static_pointer_cast<Ast::OprNode>(root), func, mod);
        return ir;
    }
    case NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_binary_instr(Ir::INSTR_ADD, IMM_I32, Ir::make_constant(0), Ir::make_constant(r->imm));
        func->add_instr(res);
        return res;
    }
    case NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        auto to = find_value(r, func, mod);
        return to;
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

void Convertor::analyze_statement_node(pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_left_value(r, func, mod);
        func->add_instr(Ir::make_store_instr(to->tr, to, analyze_value(r->val, func, mod)));
        break;
    }
    case NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        func->add_instr(tmp = Ir::make_alloc_instr(r->var.tr));
        func->add_instr(Ir::make_store_instr(r->var.tr, tmp, analyze_value(r->val, func, mod)));
        env()->set_var(r->var.sym, tmp);
        break;
    }
    case NODE_OPR:
        analyze_value(root, func, mod);
        break;
    case NODE_BLOCK: {
        auto r = std::static_pointer_cast<Ast::BlockNode>(root);
        push_env();
        for(auto i : r->body) {
            analyze_statement_node(i, func, mod);
        }
        end_env();
        break;
    }
    case NODE_IMM: // meaningless
    case NODE_SYM: // meaningless
        break;
    case NODE_DEF_FUNC:
        // impossible
        node_assert(false, root, "[Convertor] function nested - impossible");
    }
}

void Convertor::generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod)
{
    push_env();
    Ir::pFuncDefined func;
    {
        func = Ir::make_func_defined(root->var, root->args);
        set_func(func->var.sym, func);
        my_assert(root->args.size() == func->args_value.size(), "Error: inner error.");
        for(size_t i=0; i<root->args.size(); ++i) {
            env()->set_var(root->args[i].sym, func->args_value[i]);
        }
    }
    analyze_statement_node(root->body, func, mod);
    mod->add_func(func);
    end_env();
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod)
{
    push_env();
    if(root->val->type != NODE_IMM) {
        node_assert(false, root, "[Convertor] error 4: expression outside a function");
    }
    mod->add_global(Ir::make_global(root->var, std::static_pointer_cast<Ast::ImmNode>(root->val)->imm));
    end_env();
}

void Convertor::generate_single(pNode root, Ir::pModule mod)
{
    switch(root->type) {
    case NODE_DEF_FUNC:
        generate_function(std::static_pointer_cast<Ast::FuncDefNode>(root), mod);
        break;
    case NODE_DEF_VAR:
        generate_global_var(std::static_pointer_cast<Ast::VarDefNode>(root), mod);
        break;
    default:
        node_assert(false, root, "[Convertor] error 5: global operation has type that not implemented");
    }
}

Ir::pModule Convertor::generate(AstProg asts)
{
    clear_env();
    clear_loop_env();
    Ir::pModule mod = Ir::pModule(new Ir::Module());
    for(auto i : asts) {
        generate_single(i, mod);
    }
    return mod;
}

Ir::BinInstrType Convertor::fromBinaryOpr(Pointer<Ast::OprNode> root)
{
#define SELECT(x) case OPR_##x: return Ir::INSTR_##x;
    switch(root->type) {
    SELECT(ADD)
    SELECT(SUB)
    SELECT(MUL)
    SELECT(SDIV)
    SELECT(REM)
    SELECT(AND)
    SELECT(OR)
    default:
        node_assert(false, root, "[Convertor] error 6: binary operation conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::INSTR_ADD;
}

Ir::CmpType Convertor::fromCmpOpr(Pointer<Ast::OprNode> root)
{
#define SELECT(x) case OPR_##x: return Ir::CMP_##x;
    switch(root->type) {
    SELECT(EQ)
    SELECT(NE)
    SELECT(UGT)
    SELECT(UGE)
    SELECT(ULT)
    SELECT(ULE)
    SELECT(SGT)
    SELECT(SGE)
    SELECT(SLT)
    SELECT(SLE)
    default:
        node_assert(false, std::static_pointer_cast<Node>(root), "[Convertor] error 7: comparasion operation conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::CMP_EQ;
}

pEnv Convertor::env() {
    if(_env_stack.empty())
        return pEnv();
    return _env_stack.top();
}

void Convertor::push_env() {
    _env_stack.push(pEnv(new Env(env())));
}
    
void Convertor::end_env() {
    _env_stack.pop();
}
    
void Convertor::clear_env() {
    while(!_env_stack.empty())
        _env_stack.pop();
}

pLoopEnv Convertor::loop_env() {
    if(_env_stack.empty())
        return pLoopEnv();
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

