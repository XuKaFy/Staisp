#include "convert_ast_to_ir.h"

namespace AstToIr {

void Convertor::node_assert(bool judge, Ast::pNode root, Symbol message)
{
    if(!judge) {
        Staisp::error_at_token(root->token, message);
    }
}

Ir::pInstr Convertor::find_left_value(Pointer<Ast::AssignNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    if(var_map.count(root->sym)) { // local
        return var_map[root->sym]->tr, var_map[root->sym];
    } else { // global
        for(auto i : mod->globs) {
            if(strcmp(i->val.sym, root->sym) == 0) {
                return Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
            }
        }
    }
    node_assert(false, root, "[Convertor] error 1: left value cannot be found");
    return Ir::make_label_instr();
}

Ir::pInstr Convertor::find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    if(var_map.count(root->sym)) { // local
        return func->add_instr(Ir::make_load_instr(var_map[root->sym]->tr, var_map[root->sym]));
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
    return Ir::make_label_instr();
}

Ir::pInstr Convertor::analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::OPR_ADD:
    case Ast::OPR_SUB:
    case Ast::OPR_MUL:
    case Ast::OPR_SDIV:
    case Ast::OPR_AND:
    case Ast::OPR_OR:
    case Ast::OPR_REM: {
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
    case Ast::OPR_EQ:
    case Ast::OPR_NE:
    case Ast::OPR_UGT:
    case Ast::OPR_UGE:
    case Ast::OPR_ULT:
    case Ast::OPR_ULE:
    case Ast::OPR_SGT:
    case Ast::OPR_SGE:
    case Ast::OPR_SLT:
    case Ast::OPR_SLE: {
        // impossible
        node_assert(root->ch.size() == 2, root, "[Convertor] binary opr has not 2 args - impossible");
        auto a = root->ch.begin();
        auto a1 = analyze_value(*(a++), func, mod);
        auto a2 = analyze_value(*(a++), func, mod);
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root), join_type(a1->tr, a2->tr), a1, a2);
        func->add_instr(ir);
        return ir;
    }
    case Ast::OPR_IF: {
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
        return Ir::make_label_instr();
    }
    case Ast::OPR_WHILE: {
        auto compBlock = Ir::make_block();
        auto trueBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();
        // impossible
        node_assert(root->ch.size() == 3 || root->ch.size() == 2, root, "[Convertor] \"WHILE\" opr has not 2 args - impossible");

        auto a = root->ch.begin();

        func->current_block()->finish_block_with_jump(compBlock);
        
        func->add_block(compBlock);
        auto cond = analyze_value(*(a++), func, mod);
        compBlock->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));

        func->add_block(trueBlock);
        analyze_statement_node(*(a++), func, mod);   
        trueBlock->finish_block_with_jump(compBlock);
        
        func->add_block(afterBlock);
        return Ir::make_label_instr();
    }
    case Ast::OPR_RET: {
        // impossible
        node_assert(root->ch.size() == 1, root, "[Convertor] \"RET\" opr has not 1 args - impossible");
        auto a = analyze_value(root->ch.front(), func, mod);
        func->add_instr(Ir::make_ret_instr(a->tr, a));
        return Ir::make_label_instr();
    }
    case Ast::OPR_CALL: {
        // impossible
        node_assert(root->ch.size() >= 1, root, "[Convertor] \"CALL\" opr has less than 1 args - impossible");
        Vector<Ir::pInstr> args;
        // impossible
        node_assert(root->ch.front()->type == Ast::NODE_SYM, root, "[Convertor] \"CALL\" opr gets a name that not sym - impossible");
        for(auto i=++root->ch.begin(); i!=root->ch.end(); ++i) {
            args.push_back(analyze_value(*i, func, mod));
        }
        auto name_node = std::static_pointer_cast<Ast::SymNode>(root->ch.front());
        auto func_instr = func_map[name_node->sym];
        auto func_ir = Ir::make_call_instr(func_instr, args);
        func->add_instr(func_ir);
        return func_ir;
    }
    default:
        node_assert(false, root, "[Convertor] error 2: operation not implemented");
        return Ir::make_label_instr();
    }
}

Ir::pInstr Convertor::analyze_value(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_OPR: {
        auto ir = analyze_opr(std::static_pointer_cast<Ast::OprNode>(root), func, mod);
        return ir;
    }
    case Ast::NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_binary_instr(Ir::INSTR_ADD, IMM_I32, Ir::make_constant(0), Ir::make_constant(r->imm));
        func->add_instr(res);
        return res;
    }
    case Ast::NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        auto to = find_value(r, func, mod);
        return to;
    }
    case Ast::NODE_ASSIGN:
    case Ast::NODE_DEF_VAR:
    case Ast::NODE_DEF_FUNC:
    case Ast::NODE_BLOCK:
    default:
        node_assert(false, root, "[Convertor] error 3: node not calculatable");
        return Ir::make_label_instr();
    }
}

void Convertor::analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_left_value(r, func, mod);
        func->add_instr(Ir::make_store_instr(to->tr, to, analyze_value(r->val, func, mod)));
        break;
    }
    case Ast::NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        func->add_instr(tmp = Ir::make_alloc_instr(r->var.tr));
        func->add_instr(Ir::make_store_instr(r->var.tr, tmp, analyze_value(r->val, func, mod)));
        var_map[r->var.sym] = tmp;
        break;
    }
    case Ast::NODE_OPR:
        analyze_value(root, func, mod);
        break;
    case Ast::NODE_BLOCK: {
        auto r = std::static_pointer_cast<Ast::BlockNode>(root);
        for(auto i : r->body) {
            analyze_statement_node(i, func, mod);
        }
        break;
    }
    case Ast::NODE_IMM: // meaningless
    case Ast::NODE_SYM: // meaningless
        break;
    case Ast::NODE_DEF_FUNC:
        // impossible
        node_assert(false, root, "[Convertor] function nested - impossible");
    }
}

void Convertor::generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod)
{
    var_map.clear();
    Ir::pFuncDefined func;
    {
        func = Ir::make_func_defined(root->var, root->args);
        func_map[func->var.sym] = func;
        for(size_t i=0; i<root->args.size(); ++i) {
            // 2*i+1 is related to how first function block is initialized
            var_map[root->args[i].sym] = func->body[0]->instrs[2*i+1];
        }
    }
    analyze_statement_node(root->body, func, mod);
    mod->add_func(func);
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod)
{
    if(root->val->type != Ast::NODE_IMM) {
        node_assert(false, root, "[Convertor] error 4: expression outside a function");
    }
    mod->add_global(Ir::make_global(root->var, std::static_pointer_cast<Ast::ImmNode>(root->val)->imm));
}

void Convertor::generate_single(Ast::pNode root, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_DEF_FUNC:
        generate_function(std::static_pointer_cast<Ast::FuncDefNode>(root), mod);
        break;
    case Ast::NODE_DEF_VAR:
        generate_global_var(std::static_pointer_cast<Ast::VarDefNode>(root), mod);
        break;
    default:
        node_assert(false, root, "[Convertor] error 5: global operation has type that not implemented");
    }
}

Ir::pModule Convertor::generate(Ast::AstProg asts)
{
    func_map.clear();
    Ir::pModule mod = Ir::pModule(new Ir::Module());
    for(auto i : asts) {
        generate_single(i, mod);
    }
    return mod;
}

Ir::BinInstrType Convertor::fromBinaryOpr(Pointer<Ast::OprNode> root)
{
#define SELECT(x) case Ast::OPR_##x: return Ir::INSTR_##x;
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
#define SELECT(x) case Ast::OPR_##x: return Ir::CMP_##x;
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
        node_assert(false, std::static_pointer_cast<Ast::Node>(root), "[Convertor] error 7: comparasion operation conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::CMP_EQ;
}

} // namespace ast_to_ir

