#include "convert_ast_to_ir.h"

namespace AstToIr {

void Convertor::throw_error(pNode root, int id, Symbol msg)
{
    root->throw_error(id, "Convertor", msg);
}

Ir::pModule Convertor::module() const
{
    return _mod;
}

Ir::pInstr Convertor::find_left_value(pNode root, Symbol sym, Ir::pFuncDefined func)
{
    if(_env.env()->count(sym)) { // local
        Ir::pInstr found = _env.env()->find(sym);
        // const value is error
        if(!is_pointer(found->tr))
            throw_error(root, 10, "assignment to a local const value");
        // no-const value should be loaded
        return found;
    } else { // global
        for(auto i : module()->globs) {
            if(strcmp(i->val.sym, sym) == 0) {
                if(is_const_type(i->val.tr)) {
                    throw_error(root, 11, "assignment to a global const value");
                }
                return Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
            }
        }
    }
    printf("Warning: left-value \"%s\" not found.\n", sym);
    throw_error(root, 1, "left value cannot be found");
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
        auto array = find_left_value(r, r->v, func);
        Vector<Ir::pInstr> indexs;
        for(auto i : r->index)
            indexs.push_back(analyze_value(i, func));
        auto res = Ir::make_item_instr(array, indexs);
        return func->add_instr(res);
    }
    if(lv->type != NODE_SYM) {
        throw_error(lv, 12, "expected to be a left-value");
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
    throw_error(root, -1, "symbol cannot be found - impossible");
    return Ir::make_empty_instr();
}

Ir::pInstr Convertor::cast_to_type(pNode root, Ir::pInstr val, pType tr, Ir::pFuncDefined func)
{
    if(is_same_type(val->tr, tr)) return val;
    if(!is_castable(val->tr, tr)) {
        printf("Message: not castable from %s to %s\n", val->tr->type_name(), tr->type_name());
        throw_error(root, 13, "[Convertor] error 13: not castable");
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
        if(root->ch.size() != 2)
            throw_error(root, -1, "binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto an1 = *a;
        auto a1 = analyze_value(*(a++), func);
        auto an2 = *a;
        auto a2 = analyze_value(*(a++), func);
        auto joined_type = join_type(a1->tr, a2->tr);
        if(!joined_type) {
            printf("Warning: try to join %s and %s.\n", a1->tr->type_name(), a2->tr->type_name());
            throw_error(root, 18, "type has no joined type");
        }
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
        if(root->ch.size() != 2)
            throw_error(root, -1, "binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto an1 = *a;
        auto a1 = analyze_value(*(a++), func);
        auto an2 = *a;
        auto a2 = analyze_value(*(a++), func);
        auto ty = join_type(a1->tr, a2->tr);
        if(!ty)
            throw_error(root, 18, "type has no joined type");
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root, ty), ty, cast_to_type(an1, a1, ty, func), cast_to_type(an2, a2, ty, func));
        func->add_instr(ir);
        return ir;
    }
    case OPR_IF: {
        auto trueBlock = Ir::make_block();
        auto falseBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();
        // impossible
        if(root->ch.size() != 3 && root->ch.size() != 2)
            throw_error(root, -1, "\"IF\" has not 2 or 3 args - impossible");
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
        if(root->ch.size() != 2)
            throw_error(root, -1, "\"WHILE\" has not 2 args - impossible");

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
        if(root->ch.size() != 1)
            throw_error(root, -1, "\"RET\" opr has not 1 args - impossible");
        auto a = analyze_value(root->ch.front(), func);
        func->add_instr(Ir::make_ret_instr(func->var.tr, cast_to_type(root->ch.front(), a, func->var.tr, func)));
        auto nextBlock = Ir::make_block();
        func->add_block(nextBlock);
        return Ir::make_empty_instr();
    }
    case OPR_CALL: {
        // impossible
        if(root->ch.size() < 1)
            throw_error(root, -1, "\"CALL\" opr has less than 1 args - impossible");
        Vector<Ir::pInstr> args;
        // impossible
        if(root->ch.front()->type != NODE_SYM)
            throw_error(root, -1, "\"CALL\" opr gets a name that not sym - impossible");
        for(auto i=++root->ch.begin(); i!=root->ch.end(); ++i) {
            args.push_back(analyze_value(*i, func));
        }
        auto name_node = std::static_pointer_cast<Ast::SymNode>(root->ch.front());
        auto func_instr = find_func(name_node->sym);
        if(func_instr->args.size() != root->ch.size() - 1)
            throw_error(root, 8, "wrong count of arguments");
        auto func_ir = Ir::make_call_instr(func_instr, args);
        func->add_instr(func_ir);
        return func_ir;
    }
    case OPR_BREAK: {
        // impossible
        if(root->ch.size() != 0)
            throw_error(root, -1, "\"BREAK\" opr has args - impossible");
        if(!has_loop_env())
            throw_error(root, 9, "no outer loops");
        
        auto r = Ir::make_br_instr(loop_env()->loop_end->label());
        func->add_instr(r);
        auto nextBlock = Ir::make_block();
        func->add_block(nextBlock);
        return Ir::make_empty_instr();
    }
    case OPR_CONTINUE: {
        // impossible
        if(root->ch.size() != 0)
            throw_error(root, -1, "\"CONTINUE\" opr has args - impossible");
        if(!has_loop_env())
            throw_error(root, 9, "no outer loops");
        
        auto r = Ir::make_br_instr(loop_env()->loop_begin->label());
        func->add_instr(r);
        auto nextBlock = Ir::make_block();
        func->add_block(nextBlock);
        return Ir::make_empty_instr();
    }
    default:
        throw_error(root, 2, "operation not implemented");
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
        auto array = find_left_value(r, r->v, func);
        Vector<Ir::pInstr> indexs;
        for(auto i : r->index) {
            indexs.push_back(analyze_value(i, func));
            if(!is_integer(indexs.back()->tr))
                throw_error(i, 14, "type of index should be integer");
        }
        auto itemptr = Ir::make_item_instr(array, indexs);
        func->add_instr(itemptr);
        return itemptr;
    }
    case NODE_ARRAY_VAL: {
        throw_error(root, -1, "array definition is not calculatable - impossible");
    }
    case NODE_ASSIGN:
    case NODE_DEF_VAR:
    case NODE_DEF_FUNC:
    case NODE_BLOCK:
    default:
        throw_error(root, 3, "node not calculatable");
        return Ir::make_empty_instr();
    }
}

void Convertor::copy_to_array(pNode root, Ir::pInstr addr, pType cur_type, Vector<pNode> nums, Vector<Ir::pInstr> indexs, Ir::pFuncDefined func)
{
    if(!is_pointer(addr->tr)) {
        throw_error(root, 16, "list should initialize type that is pointed");
    }
    auto array_type = std::static_pointer_cast<ArrayType>(to_pointed_type(cur_type));
    auto elem_type = array_type->elem_type;
    auto subarray_type = make_pointer_type(elem_type, false);
    size_t valid_size = std::min(array_type->elem_count, nums.size());
    if(is_array(elem_type)) {
        for(size_t i=0; i<valid_size; ++i) {
            if(nums[i]->type != NODE_ARRAY_VAL)
                throw_error(root, 17, "list doesn't match the expected value");
            auto index = Ir::make_binary_instr(Ir::INSTR_ADD, make_basic_type(IMM_I32, false), Ir::make_constant(ImmValue(0ull, IMM_I32)), Ir::make_constant(ImmValue((unsigned long long)i, IMM_I32)));
            indexs.push_back(index);
            func->add_instr(index);
            copy_to_array(root, addr, subarray_type, std::static_pointer_cast<Ast::ArrayDefNode>(nums[i])->nums, indexs, func);
            indexs.pop_back();
        }
        return ;
    }
    for(size_t i=0; i<valid_size; ++i) {
        auto index = Ir::make_binary_instr(Ir::INSTR_ADD, make_basic_type(IMM_I32, false), Ir::make_constant(ImmValue(0ull, IMM_I32)), Ir::make_constant(ImmValue((unsigned long long)i, IMM_I32)));
        indexs.push_back(index);
        auto item = Ir::make_item_instr(addr, indexs);
        
        func->add_instr(index);
        func->add_instr(item);
        
        auto store = Ir::make_store_instr(item, analyze_value(nums[i], func));
        func->add_instr(store);
        indexs.pop_back();
    }
}

void Convertor::analyze_statement_node(pNode root, Ir::pFuncDefined func)
{
    switch(root->type) {
    case NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_left_value(r, r->lv, func);
        if(!is_pointer(to->tr))
            throw_error(r, 19, "try to dereference a non-pointer value");
        auto j = to_pointed_type(to->tr);
        func->add_instr(Ir::make_store_instr(to, cast_to_type(r->val, analyze_value(r->val, func), j, func)));
        break;
    }
    case NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        if(is_array(r->var.tr)) {
            if(r->val->type != NODE_ARRAY_VAL)
                throw_error(r->val, 17, "array should be initialized by a list");
            auto rrr = std::static_pointer_cast<Ast::ArrayDefNode>(r->val);
            func->add_instr(tmp = Ir::make_alloc_instr(r->var.tr));
            _env.env()->set(r->var.sym, tmp);
            copy_to_array(r, tmp, tmp->tr, rrr->nums, {}, func);
            break;
        }
        if(is_const_type(r->var.tr)) {
            _env.env()->set(r->var.sym, cast_to_type(r->val, analyze_value(r->val, func), r->var.tr, func));
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
        throw_error(root, -1, "statement calculation not implemented - impossible");
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
    module()->add_global(Ir::make_global(root->var, *Ast::Executor(_prog).must_have_value_execute(root->val)));
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
        throw_error(root, 5, "global operation has type that not implemented");
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
        throw_error(root, -1, "binary operation conversion from ast to ir not implemented");
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
        throw_error(root, 7, "comparasion operation conversion from ast to ir not implemented");
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

