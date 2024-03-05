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

    bool var_count(Symbol sym) {
        if(_var_map.count(sym)) return true;
        if(_parent) return _parent->var_count(sym);
        return false;
    }

    Ir::pInstr find_var(Symbol sym) {
        if(_var_map.count(sym))
            return _var_map[sym];
        return _parent->find_var(sym);
    }

    void set_var(Symbol sym, Ir::pInstr i) {
        _var_map[sym] = i;
    }

private:
    pEnv _parent;
    Map<String, Ir::pInstr> _var_map;
};

typedef Stack<pEnv> EnvStack;

class Convertor {
public:
    Ir::pModule generate(Ast::AstProg asts);
    static Ir::BinInstrType fromBinaryOpr(Pointer<Ast::OprNode> root);
    static Ir::CmpType fromCmpOpr(Pointer<Ast::OprNode> root);

private:
    void generate_single(Ast::pNode root, Ir::pModule mod);
    void generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod);
    void generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod);
    void analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr analyze_value(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr analyze_opr(Pointer<Ast::OprNode> root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr find_value(Pointer<Ast::SymNode> root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr find_left_value(Pointer<Ast::AssignNode> root, Ir::pFuncDefined func, Ir::pModule mod);

    static void node_assert(bool judge, Ast::pNode root, Symbol message);

    pEnv env() {
        if(_env_stack.empty())
            return pEnv();
        return _env_stack.top();
    }

    void push_env() {
        _env_stack.push(pEnv(new Env(env())));
    }
    
    void end_env() {
        _env_stack.pop();
    }
    
    void clear_env() {
        while(!_env_stack.empty())
            _env_stack.pop();
    }

    void set_func(Symbol sym, Ir::pFunc fun) {
        _func_map[sym] = fun;
    }

    bool func_count(Symbol sym) {
        return _func_map.count(sym);
    }

    Ir::pFunc find_func(Symbol sym) {
        return _func_map[sym];
    }

    EnvStack _env_stack;
    Map<String, Ir::pFunc> _func_map;
};


} // namespace ast_to_ir
