#include "convert_ast_to_ir.h"

#include "ir_call_instr.h"
#include "ir_cast_instr.h"
#include "ir_ptr_instr.h"
#include "ir_control_instr.h"
#include "ir_mem_instr.h"

#include "imm.h"
#include "value.h"

namespace AstToIr {

void Convertor::throw_error(pNode root, int id, Symbol msg)
{
    root->throw_error(id, "Convertor", msg);
}

Ir::pModule Convertor::module() const
{
    return _mod;
}

Ir::pInstr Convertor::add_instr(Ir::pInstr instr)
{
    _cur_func->add_body(instr);
    return instr;
}

Ir::pVal Convertor::find_left_value(pNode root, Symbol sym)
{
    if(_env.env()->count(sym)) { // local
        Ir::pInstr found = _env.env()->find(sym);
        // const value is error
        if(!is_pointer(found->ty))
            throw_error(root, 10, "assignment to a local const value");
        // no-const value should be loaded
        return found;
    } else { // global
        for(auto i : module()->globs) {
            if(strcmp(i->name()+1, sym) == 0) {
                auto sym_node = Ir::make_sym_instr(TypedSym(to_symbol(String("@") + sym), i->ty));
                _cur_func->add_imm(sym_node);
                return sym_node;
            }
        }
    }
    printf("Warning: left-value \"%s\" not found.\n", sym);
    throw_error(root, 1, "left value cannot be found");
    return Ir::make_empty_instr();
}

Ir::pVal Convertor::find_left_value(pNode lv)
{
    if(lv->type == NODE_DEREF) {
        auto r = std::static_pointer_cast<Ast::DerefNode>(lv);
        auto res = analyze_value(r->val);
        return res;
    }
    if(lv->type == NODE_ITEM) {
        auto r = std::static_pointer_cast<Ast::ItemNode>(lv);
        auto array = find_left_value(r, r->v);
        Vector<Ir::pVal> indexs;
        for(auto i : r->index)
            indexs.push_back(analyze_value(i));
        auto res = Ir::make_item_instr(array, indexs);
        return add_instr(res);
    }
    if(lv->type != NODE_SYM) {
        throw_error(lv, 12, "expected to be a left-value");
    }
    Symbol sym = std::static_pointer_cast<Ast::SymNode>(lv)->sym;
    return find_left_value(lv, sym);
}

Ir::pVal Convertor::find_value(Pointer<Ast::SymNode> root)
{
    if(_env.env()->count(root->sym)) { // local
        Ir::pInstr found = _env.env()->find(root->sym);
        // in this implement, all local value is stored
        return add_instr(Ir::make_load_instr(found));
    } else { // global
        for(auto i : module()->globs) {
            if(strcmp(i->name()+1, root->sym) == 0) { // global name: "@xxx"
                auto from = Ir::make_sym_instr(TypedSym(to_symbol(String("@") + root->sym), i->ty));
                _cur_func->add_imm(from);
                return add_instr(Ir::make_load_instr(from));
            }
        }
    }
    // impossible
    throw_error(root, -1, "symbol cannot be found - impossible");
    return Ir::make_empty_instr();
}

Ir::pVal Convertor::cast_to_type(pNode root, Ir::pVal val, pType ty)
{
    if(is_same_type(val->ty, ty)) return val;
    if(!is_castable(val->ty, ty)) {
        printf("Message: not castable from %s to %s\n", val->ty->type_name(), ty->type_name());
        throw_error(root, 13, "[Convertor] error 13: not castable");
    }
    auto r = Ir::make_cast_instr(ty, val);
    add_instr(r);
    return r;
}

Ir::pVal Convertor::analyze_opr(Pointer<Ast::OprNode> root)
{
    switch(root->type) {
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MUL:
    case OPR_DIV:
    case OPR_AND:
    case OPR_OR:
    case OPR_REM: {
        if(root->ch.size() != 2)
            throw_error(root, -1, "binary opr has not 2 args - impossible");
        auto a1 = analyze_value(root->ch[0]);
        auto a2 = analyze_value(root->ch[1]);
        auto joined_type = join_type(a1->ty, a2->ty);
        if(!joined_type) {
            printf("Warning: try to join %s and %s.\n", a1->ty->type_name(), a2->ty->type_name());
            throw_error(root, 18, "type has no joined type");
        }
        auto ir = Ir::make_binary_instr(fromBinaryOpr(root), cast_to_type(root->ch[0], a1, joined_type), cast_to_type(root->ch[1], a2, joined_type));
        add_instr(ir);
        return ir;
    }
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE: {
        if(root->ch.size() != 2)
            throw_error(root, -1, "binary opr has not 2 args - impossible");
        auto a1 = analyze_value(root->ch[0]);
        auto a2 = analyze_value(root->ch[1]);
        auto ty = join_type(a1->ty, a2->ty);
        if(!ty)
            throw_error(root, 18, "type has no joined type");
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root, ty), cast_to_type(root->ch[0], a1, ty), cast_to_type(root->ch[1], a2, ty));
        add_instr(ir);
        return ir;
    }
    default:
        throw_error(root, 2, "operation not implemented");
        return Ir::make_empty_instr();
    }
}

Ir::pVal Convertor::analyze_value(pNode root)
{
    switch(root->type) {
    case NODE_CAST: {
        auto r = std::static_pointer_cast<Ast::CastNode>(root);
        auto res = Ir::make_cast_instr(r->ty, analyze_value(r->val));
        add_instr(res);
        return res;
    }
    case NODE_OPR: {
        auto ir = analyze_opr(std::static_pointer_cast<Ast::OprNode>(root));
        return ir;
    }
    case NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_constant(r->imm);
        _cur_func->add_imm(res);
        return res;
    }
    case NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        auto to = find_value(r);
        return to;
    }
    case NODE_REF: {
        auto r = std::static_pointer_cast<Ast::RefNode>(root);
        return find_left_value(root, r->name);
    }
    case NODE_DEREF: {
        auto r = std::static_pointer_cast<Ast::DerefNode>(root);
        auto res = Ir::make_load_instr(analyze_value(r->val));
        add_instr(res);
        return res;
    }
    case NODE_ITEM: {
        auto r = std::static_pointer_cast<Ast::ItemNode>(root);
        auto array = find_left_value(r, r->v);
        Vector<Ir::pVal> indexs;
        for(auto i : r->index) {
            indexs.push_back(analyze_value(i));
            if(!is_integer(indexs.back()->ty))
                throw_error(i, 14, "type of index should be integer");
        }
        auto itemptr = Ir::make_item_instr(array, indexs);
        add_instr(itemptr);
        return itemptr;
    }
    case NODE_ARRAY_VAL: {
        throw_error(root, -1, "array definition is not calculatable - impossible");
    }
    case NODE_CALL: {
        auto r = std::static_pointer_cast<Ast::CallNode>(root);
        if(!func_count(r->name)) {
            throw_error(r, 20, "function not found");
        }
        auto func = find_func(r->name);
        Vector<Ir::pVal> args;
        // assert size equal
        size_t length = func->functon_type()->arg_type.size();
        if(length != r->ch.size())
            throw_error(root, 8, "wrong count of arguments");
        for(size_t i=0; i<length; ++i) {
            Ir::pVal cur_arg = analyze_value(r->ch[i]);
            args.push_back(cast_to_type(r->ch[i], cur_arg, func->functon_type()->arg_type[i]));
        }
        return add_instr(Ir::make_call_instr(func, args));
    }
    case NODE_ASSIGN:
    case NODE_DEF_VAR:
    case NODE_IF:
    case NODE_WHILE:
    case NODE_FOR:
    case NODE_CONTINUE:
    case NODE_BREAK:
    case NODE_RETURN:
    case NODE_BLOCK:
    case NODE_DEF_FUNC:
        throw_error(root, 3, "node not calculatable");
    }
    return Ir::make_empty_instr();
}

void Convertor::copy_to_array(pNode root, Ir::pInstr addr, pType cur_type, Vector<pNode> nums, Vector<Ir::pVal> indexs)
{
    if(!is_pointer(addr->ty)) {
        throw_error(root, 16, "list should initialize type that is pointed");
    }
    auto array_type = std::static_pointer_cast<ArrayType>(to_pointed_type(cur_type));
    auto elem_type = array_type->elem_type;
    auto subarray_type = make_pointer_type(elem_type);
    size_t valid_size = std::min(array_type->elem_count, nums.size());
    if(is_array(elem_type)) {
        for(size_t i=0; i<valid_size; ++i) {
            if(nums[i]->type != NODE_ARRAY_VAL)
                throw_error(root, 17, "list doesn't match the expected value");
            auto index = Ir::make_constant(ImmValue((unsigned long long)i, IMM_I32));
            _cur_func->add_imm(index);
            indexs.push_back(index);
            copy_to_array(root, addr, subarray_type, std::static_pointer_cast<Ast::ArrayDefNode>(nums[i])->nums, indexs);
            indexs.pop_back();
        }
        return ;
    }
    for(size_t i=0; i<valid_size; ++i) {
        auto index = Ir::make_constant(ImmValue((unsigned long long)i, IMM_I32));
        indexs.push_back(index);
        _cur_func->add_imm(index);
        auto item = Ir::make_item_instr(addr, indexs);
        
        add_instr(item);
        
        auto store = Ir::make_store_instr(item, analyze_value(nums[i]));
        add_instr(store);
        indexs.pop_back();
    }
}

void Convertor::analyze_statement_node(pNode root)
{
    switch(root->type) {
    case NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto lv = find_left_value(r->lv);
        add_instr(Ir::make_store_instr(lv, cast_to_type(r->val, analyze_value(r->val), to_pointed_type(lv->ty))));
        break;
    }
    case NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        if(is_array(r->var.ty)) {
            if(r->val->type != NODE_ARRAY_VAL)
                throw_error(r->val, 17, "array should be initialized by a list");
            auto rrr = std::static_pointer_cast<Ast::ArrayDefNode>(r->val);
            add_instr(tmp = Ir::make_alloc_instr(r->var.ty));
            _env.env()->set(r->var.sym, tmp);
            copy_to_array(r, tmp, tmp->ty, rrr->nums);
            break;
        }
        add_instr(tmp = Ir::make_alloc_instr(r->var.ty));
        add_instr(Ir::make_store_instr(tmp, cast_to_type(r->val, analyze_value(r->val), r->var.ty)));
        _env.env()->set(r->var.sym, tmp);
        break;
    }
    case NODE_IF: {
        auto r = std::static_pointer_cast<Ast::IfNode>(root);
        auto if_begin = Ir::make_label_instr();
        auto if_end = Ir::make_label_instr();
        auto if_else_end = Ir::make_label_instr();
        if(r->elsed) {
            /*
                br xxx IF_BEGIN, ELSE_BEGIN
                IF_BEGIN:
                    XXX
                    GOTO ELSE_END:
                IF_END/ELSE_BEGIN:
                    YYY
                    GOTO ELSE_END:
                ELSE_END:
            */
            add_instr(Ir::make_br_cond_instr(cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1)), if_begin, if_end));
            add_instr(if_begin);
            analyze_statement_node(r->body);
            if(_cur_func->body.size() && !is_ir_type(_cur_func->body.back()->ty, IR_RET)) {
                add_instr(Ir::make_br_instr(if_else_end));
            }
            add_instr(if_end);
            analyze_statement_node(r->elsed);
            if(_cur_func->body.size() && !is_ir_type(_cur_func->body.back()->ty, IR_RET)) {
                add_instr(Ir::make_br_instr(if_else_end));
            }
            add_instr(if_else_end);
        } else {
            /*
                br xxx IF_BEGIN, IF_END
                IF_BEGIN:
                    XXX
                    GOTO IF_END:
                IF_END:
            */
            add_instr(Ir::make_br_cond_instr(cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1)), if_begin, if_end));
            add_instr(if_begin);
            analyze_statement_node(r->body);
            if(_cur_func->body.size() && !is_ir_type(_cur_func->body.back()->ty, IR_RET)) {
                add_instr(Ir::make_br_instr(if_end));
            }
            add_instr(if_end);
        }
        break;
    }
    case NODE_WHILE: {
        auto r = std::static_pointer_cast<Ast::WhileNode>(root);
        /*
            br while_cond
            while_cond:
                br ... LABEL while_begin, LABEL while_end
            while_begin:
                ...
                br while_cond
            while_end:
        */
        auto while_cond = Ir::make_label_instr();
        auto while_begin = Ir::make_label_instr();
        auto while_end = Ir::make_label_instr();
        add_instr(Ir::make_br_instr(while_cond));
        add_instr(while_cond);
        auto cond = cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1));
        add_instr(Ir::make_br_cond_instr(cond, while_begin, while_end));
        add_instr(while_begin);
        push_loop_env(while_cond, while_end);
        analyze_statement_node(r->body);
        // add_instr(Ir::make_br_instr(while_cond));
        if(_cur_func->body.size() && !is_ir_type(_cur_func->body.back()->ty, IR_RET)) {
            add_instr(Ir::make_br_instr(while_cond)); // who will write "while(xxx) return 0"?
        }
        add_instr(while_end);
        end_loop_env();
        break;
    }
    case NODE_FOR: {
        /*
                some_init...
                br for_cond
            for_cond:
                some_program...
                br ... LABEL for_body, LABEL for_end
            for_body:
                some_program...
                br for_exec
            for_exec:
                some_exec...
                br for_cond
            for_end:
        */
        auto r = std::static_pointer_cast<Ast::ForNode>(root);
        auto for_cond = Ir::make_label_instr();
        auto for_body = Ir::make_label_instr();
        auto for_exec = Ir::make_label_instr();
        auto for_end = Ir::make_label_instr();
        _env.push_env();
        push_loop_env(for_exec, for_end); // continue start from for_exec
        analyze_statement_node(r->init);
        add_instr(Ir::make_br_instr(for_cond));
        add_instr(for_cond);
        auto cond = analyze_value(r->cond);
        add_instr(Ir::make_br_cond_instr(cond, for_body, for_end));
        add_instr(for_body);
        analyze_statement_node(r->body);
        add_instr(Ir::make_br_instr(for_exec));
        add_instr(for_exec);
        analyze_statement_node(r->exec);
        add_instr(Ir::make_br_instr(for_cond));
        add_instr(for_end);
        end_loop_env();
        _env.end_env();
        break;
    }
    case NODE_BREAK:
        if(!has_loop_env()) {
            throw_error(root, 9, "no outer loops");
        }
        add_instr(Ir::make_br_instr(loop_env()->loop_end));
        add_instr(Ir::make_label_instr());
        break;
    case NODE_CONTINUE:
        if(!has_loop_env()) {
            throw_error(root, 9, "no outer loops");
        }
        add_instr(Ir::make_br_instr(loop_env()->loop_begin));
        add_instr(Ir::make_label_instr());
        break;
    case NODE_RETURN: {
        auto r = std::static_pointer_cast<Ast::ReturnNode>(root);
        if(r->ret) {
            auto res = analyze_value(r->ret);
            add_instr(Ir::make_ret_instr(cast_to_type(r, res, _cur_func->functon_type()->ret_type)));
        } else {
            add_instr(Ir::make_ret_instr());
        }
        break;
    }
    case NODE_BLOCK: {
        auto r = std::static_pointer_cast<Ast::BlockNode>(root);
        _env.push_env();
        for(auto i : r->body) {
            analyze_statement_node(i);
        }
        _env.end_env();
        break;
    }
    case NODE_OPR:
    case NODE_ITEM:
    case NODE_CAST:
    case NODE_IMM:
    case NODE_SYM:
    case NODE_REF:
    case NODE_DEREF:
    case NODE_ARRAY_VAL:
    case NODE_CALL:
        analyze_value(root);
        break;
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
        Vector<pType> types;
        Vector<Symbol> syms;
        for(auto i : root->args) {
            types.push_back(i.ty);
            syms.push_back(i.sym);
        }
        func = Ir::make_func_defined(root->var, types, syms);
        for(size_t i=0; i<root->args.size(); ++i) {
            _env.env()->set(root->args[i].sym, func->args[i]);
        }
        set_func(func->name(), func);
    }
    _cur_func = func;
    analyze_statement_node(root->body);
    func->end_function();
    module()->add_func(func);
    _env.end_env();
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root)
{
    _env.push_env();
    if(root->val->type == NODE_IMM) {
        module()->add_global(Ir::make_global(root->var, 
            Value(std::static_pointer_cast<Ast::ImmNode>(root->val)->imm)));
    }
    _env.end_env();
}

void Convertor::generate_single(pNode root)
{
    switch(root->type) {
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

Ir::CmpType Convertor::fromCmpOpr(Pointer<Ast::OprNode> root, pType ty)
{
    bool is_signed = is_signed_type(ty);
    bool is_flt = is_float(ty);
    switch(root->type) {
#define SELECT(x) \
    case OPR_##x: { \
        if(is_flt) { \
            return Ir::CMP_O##x; \
        } \
        return Ir::CMP_##x; \
    }
    SELECT(EQ)
    SELECT(NE)
#undef SELECT
#define SELECT_US(x) \
    case OPR_##x: { \
        if(is_flt) { \
            return Ir::CMP_S##x; \
        } else if(is_signed) { \
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
    return _loop_env_stack.top();
}

void Convertor::push_loop_env(Ir::pInstr begin, Ir::pInstr end) {
    _loop_env_stack.push(pLoopEnv(new LoopEnv {begin, end}));
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

