#include "convert_ast_to_ir.h"

namespace AstToIr {

Ir::pInstr Convertor::find_value(Symbol sym, Ir::pModule mod)
{
    if(var_map.count(sym)) { // local
        return var_map[sym];
    } else { // global
        // TODO: no instr can modify global vars
        for(auto i : mod->globs) {
            if(strcmp(i->val.sym, sym) == 0) {
                return Ir::make_sym_instr(i->val, Ir::SYM_GLOBAL);
            }
        }
    }
    my_assert(false, "Error: symbol cannot be found");
    return Ir::make_arg_instr(IMM_VOID);
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
        my_assert(root->ch.size() == 2, "Error: binary opr of ast has not 2 args");
        auto a1 = analyze_value(root->ch[0], func, mod);
        auto a2 = analyze_value(root->ch[1], func, mod);
        // printf("[exec %d, joined_type = %d]\n", root->type, join_type(a1->tr, a2->tr));
        auto ir = Ir::make_binary_instr(fromBinaryOpr(root->type), join_type(a1->tr, a2->tr), a1, a2);
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
        my_assert(root->ch.size() == 2, "Error: binary opr of ast has not 2 args");
        auto a1 = analyze_value(root->ch[0], func, mod);
        auto a2 = analyze_value(root->ch[1], func, mod);
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root->type), join_type(a1->tr, a2->tr), a1, a2);
        func->add_instr(ir);
        return ir;
    }
    case Ast::OPR_IF: {
        auto trueBlock = Ir::make_block();
        auto falseBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();
        my_assert(root->ch.size() == 3 || root->ch.size() == 2, "Error: if opr of ast has not 2 or 3 args");
        
        auto cond = analyze_value(root->ch[0], func, mod);
        // printf("for if cond, tr = %d\n", cond->tr);
        if(root->ch.size() == 3) {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), falseBlock->label()));
        } else {
            func->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));
        }

        func->add_block(trueBlock);
        analyze_value(root->ch[1], func, mod);   
        trueBlock->finish_block_with_jump(afterBlock);
        
        if(root->ch.size() == 3) {
            func->add_block(falseBlock);
            analyze_value(root->ch[2], func, mod);
            falseBlock->finish_block_with_jump(afterBlock);
        }
        
        func->add_block(afterBlock);
        return Ir::make_arg_instr(IMM_VOID);
    }
    case Ast::OPR_WHILE: {
        auto compBlock = Ir::make_block();
        auto trueBlock = Ir::make_block();
        auto afterBlock = Ir::make_block();
        my_assert(root->ch.size() == 3 || root->ch.size() == 2, "Error: while opr of ast has not 2 args");

        func->current_block()->finish_block_with_jump(compBlock);
        
        func->add_block(compBlock);
        auto cond = analyze_value(root->ch[0], func, mod);
        compBlock->add_instr(Ir::make_br_cond_instr(cond, trueBlock->label(), afterBlock->label()));
        compBlock->finish_block_with_jump(trueBlock);

        func->add_block(trueBlock);
        analyze_value(root->ch[1], func, mod);   
        trueBlock->finish_block_with_jump(compBlock);
        
        func->add_block(afterBlock);
        return Ir::make_arg_instr(IMM_VOID);
    }
    case Ast::OPR_RET: {
        my_assert(root->ch.size() == 1, "Error: ret opr of ast has not 1 args");
        auto a = analyze_value(root->ch[0], func, mod);
        func->add_instr(Ir::make_ret_instr(a->tr, a));
        return Ir::make_arg_instr(IMM_VOID);
    }
    case Ast::OPR_CALL: {
        my_assert(root->ch.size() >= 1, "Error: call opr of ast has less than 1 args");
        Vector<Ir::pInstr> args;
        my_assert(root->ch[0]->type == Ast::NODE_SYM, "Error: call opr gets a name that not sym");
        for(size_t i=1; i<root->ch.size(); ++i) {
            args.push_back(analyze_value(root->ch[i], func, mod));
        }
        auto name_node = std::static_pointer_cast<Ast::SymNode>(root->ch[0]);
        auto func_instr = func_map[name_node->sym];
        auto func_ir = Ir::make_call_instr(func_instr, args);
        func->add_instr(func_ir);
        return func_ir;
    }
    default:
        printf("opr %d not implemented\n", root->type);
        my_assert(false, "Error: opr of ast is not implemented");
        return Ir::make_arg_instr(IMM_VOID);
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
        auto to = find_value(r->sym, mod);
        return to;
    }
    case Ast::NODE_ASSIGN:
    case Ast::NODE_DEF_VAR:
    case Ast::NODE_DEF_FUNC:
    default:
        my_assert(false, "Error: not calculatable");
        return Ir::make_arg_instr(IMM_VOID);
    }
}

void Convertor::analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod)
{
    switch(root->type) {
    case Ast::NODE_ASSIGN: {
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
        auto to = find_value(r->sym, mod);
        func->add_instr(Ir::make_store_instr(to->tr, to, analyze_value(r->val, func, mod)));
        break;
    }
    case Ast::NODE_DEF_VAR: {
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        func->add_instr(tmp = Ir::make_alloc_instr(r->var.tr));
        func->add_instr(Ir::make_store_instr(r->var.tr, tmp, Ir::make_constant(r->val)));
        var_map[r->var.sym] = tmp;
        break;
    }
    case Ast::NODE_OPR:
        analyze_value(root, func, mod);
        break;
    case Ast::NODE_IMM: // meaningless
    case Ast::NODE_SYM: // meaningless
        break;
    case Ast::NODE_DEF_FUNC:
        my_assert(false, "Error: function in function");
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
            var_map[root->args[i].sym] = func->body[0]->instrs[i];
        }
    }
    for(auto i : root->body) {
        analyze_statement_node(i, func, mod);
    }
    mod->add_func(func);
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod)
{
    mod->add_global(Ir::make_global(root->var, root->val));
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
        my_assert(false, "Error: global node has type that not implemented");
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

Ir::BinInstrType Convertor::fromBinaryOpr(Ast::OprType o)
{
#define SELECT(x) case Ast::OPR_##x: return Ir::INSTR_##x;
    switch(o) {
    SELECT(ADD)
    SELECT(SUB)
    SELECT(MUL)
    SELECT(SDIV)
    SELECT(REM)
    SELECT(AND)
    SELECT(OR)
    default:
        my_assert(false, "Error: binary opr conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::INSTR_ADD;
}

Ir::CmpType Convertor::fromCmpOpr(Ast::OprType o)
{
#define SELECT(x) case Ast::OPR_##x: return Ir::CMP_##x;
    switch(o) {
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
        my_assert(false, "Error: binary opr conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::CMP_EQ;
}

} // namespace ast_to_ir

