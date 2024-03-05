#pragma once

#include "def.h"

#include "ast_node.h"
#include "ir_module.h"
#include "ir_opr_instr.h"
#include "ir_cmp_instr.h"
#include "ir_call_instr.h"

namespace AstToIr {

struct Env;
typedef Pointer<Env> pEnv;

struct Env {
public:
    Env(pEnv parent = {})
        : _parent(parent) { }

    bool var_count(Symbol sym);
    Ir::pInstr find_var(Symbol sym);
    void set_var(Symbol sym, Ir::pInstr i);

private:
    pEnv _parent;
    Map<String, Ir::pInstr> _var_map;
};

struct LoopEnv;
typedef Pointer<LoopEnv> pLoopEnv;

struct LoopEnv {
    Ir::pBlock loop_begin;
    Ir::pBlock loop_end;
};

typedef Stack<pEnv> EnvStack;
typedef Stack<pLoopEnv> LoopEnvStack;

class Convertor {
public:
    Ir::pModule generate(AstProg asts);
    static Ir::BinInstrType fromBinaryOpr(Pointer<Ast::OprNode> root);
    static Ir::CmpType fromCmpOpr(Pointer<Ast::OprNode> root);

private:
    void generate_single(pNode root, Ir::pModule mod);
    void generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod);
    void generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod);
    void analyze_statement_node(pNode root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr analyze_value(pNode root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr find_left_value(Pointer<Ast::AssignNode> root, Ir::pFuncDefined func, Ir::pModule mod);

    static void node_assert(bool judge, pNode root, Symbol message);

    pEnv env();
    void push_env();
    void end_env();
    void clear_env();
    
    EnvStack _env_stack;
    
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
};


} // namespace ast_to_ir
