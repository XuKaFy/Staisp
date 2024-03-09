#pragma once

#include "def.h"
#include "env.h"
#include "imm.h"

#include "ast_exec.h"
#include "ast_node.h"
#include "ir_module.h"
#include "ir_opr_instr.h"
#include "ir_cmp_instr.h"
#include "ir_call_instr.h"
#include "ir_cast_instr.h"
#include "ir_ptr_instr.h"

namespace AstToIr {

struct LoopEnv;
typedef Pointer<LoopEnv> pLoopEnv;

struct LoopEnv {
    Ir::pBlock loop_begin;
    Ir::pBlock loop_end;
};

typedef Stack<pLoopEnv> LoopEnvStack;

class Convertor {
public:
    Ir::pModule generate(AstProg asts);
    static Ir::BinInstrType fromBinaryOpr(Pointer<Ast::OprNode> root);
    static Ir::CmpType fromCmpOpr(Pointer<Ast::OprNode> root, pType tr);

private:
    void generate_single(pNode root);
    void generate_global_var(Pointer<Ast::VarDefNode> root);
    void generate_function(Pointer<Ast::FuncDefNode> root);
    void analyze_statement_node(pNode root, Ir::pFuncDefined func);
    void copy_to_array(pNode root, Ir::pInstr addr, pType cur_type, Vector<pNode> nums, Vector<Ir::pInstr> indexs, Ir::pFuncDefined func);
    Ir::pInstr analyze_value(pNode root, Ir::pFuncDefined func);
    Ir::pInstr analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func);
    Ir::pInstr find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func);
    Ir::pInstr find_left_value(pNode root, pNode lv, Ir::pFuncDefined func);
    Ir::pInstr find_left_value(pNode root, Symbol name, Ir::pFuncDefined func);
    Ir::pInstr cast_to_type(pNode root, Ir::pInstr val, pType tr, Ir::pFuncDefined func);

    Ir::pModule module() const;

    EnvWrapper<Ir::pInstr> _env;
    
    void set_func(Symbol sym, Ir::pFunc fun);
    bool func_count(Symbol sym);
    Ir::pFunc find_func(Symbol sym);

    Map<String, Ir::pFunc> _func_map;

    pLoopEnv loop_env();
    void push_loop_env(pLoopEnv env);
    bool has_loop_env() const;
    void end_loop_env();
    void clear_loop_env();

    LoopEnvStack _loop_env_stack;
    Ir::pModule _mod;
    AstProg _prog;
};


} // namespace ast_to_ir
