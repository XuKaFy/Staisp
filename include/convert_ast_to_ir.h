#pragma once

#include "def.h"
#include "env.h"

#include "ast_node.h"
#include "ir_module.h"
#include "ir_opr_instr.h"
#include "ir_cmp_instr.h"

namespace AstToIr {

struct LoopEnv;
typedef Pointer<LoopEnv> pLoopEnv;

struct LoopEnv {
    Ir::pInstr loop_begin;
    Ir::pInstr loop_end;
};

typedef Stack<pLoopEnv> LoopEnvStack;

class Convertor {
public:
    Ir::pModule generate(AstProg asts);
    
    static Ir::BinInstrType fromBinaryOpr(Pointer<Ast::OprNode> root);
    static Ir::CmpType fromCmpOpr(Pointer<Ast::OprNode> root, pType tr);

private:
    static void throw_error(pNode root, int id, Symbol msg);

    void generate_single(pNode root);
    void generate_global_var(Pointer<Ast::VarDefNode> root);
    void generate_function(Pointer<Ast::FuncDefNode> root);
    void analyze_statement_node(pNode root);
    void copy_to_array(pNode root, Ir::pInstr addr, pType cur_type, Vector<pNode> nums, Vector<Ir::pVal> indexs = {});
    Ir::pVal analyze_value(pNode root);
    Ir::pVal analyze_opr(Pointer<Ast::OprNode> root);
    Ir::pVal find_value(Pointer<Ast::SymNode> root);
    Ir::pVal find_left_value(pNode lv);
    Ir::pVal find_left_value(pNode root, Symbol sym);
    Ir::pVal cast_to_type(pNode root, Ir::pVal val, pType tr);

    Ir::pModule module() const;

    Ir::pInstr add_instr(Ir::pInstr instr);

    EnvWrapper<Ir::pInstr> _env;
    
    void set_func(Symbol sym, Ir::pFunc fun);
    bool func_count(Symbol sym);
    Ir::pFunc find_func(Symbol sym);

    Map<String, Ir::pFunc> _func_map;

    pLoopEnv loop_env();
    void push_loop_env(Ir::pInstr begin, Ir::pInstr end);
    bool has_loop_env() const;
    void end_loop_env();
    void clear_loop_env();

    LoopEnvStack _loop_env_stack;
    Ir::pModule _mod;
    Ir::pFuncDefined _cur_func;
    AstProg _prog;
};


} // namespace ast_to_ir
