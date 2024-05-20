// 将 AST 转化为 LLVM IR

#pragma once

#include "def.h"
#include "env.h"

#include "ast_node.h"
#include "ir_cmp_instr.h"
#include "ir_instr.h"
#include "ir_module.h"
#include "ir_opr_instr.h"

namespace AstToIr {

struct LoopEnv;
typedef Pointer<LoopEnv> pLoopEnv;

struct LoopEnv {
    Ir::pInstr loop_begin;
    Ir::pInstr loop_end;
};

typedef Stack<pLoopEnv> LoopEnvStack;

struct MaybeConstInstr {
    Ir::pInstr instr;
    bool is_const;
};

class Convertor {
public:
    Ir::pModule generate(AstProg asts);

    static Ir::BinInstrType fromBinaryOpr(Pointer<Ast::BinaryNode> root,
                                          pType ty);
    static Ir::CmpType fromCmpOpr(Pointer<Ast::BinaryNode> root, pType tr);

private:
    static void throw_error(pNode root, int id, String msg);

    Ir::pVal find_left_value(pNode root, String sym,
                             bool request_not_const = false);
    ImmValue find_const_value(pNode root, String sym);

    ImmValue constant_eval(pNode node);

    Ir::pVal analyze_value(pNode root, bool request_not_const = false);
    Ir::pVal analyze_left_value(pNode root, bool request_not_const = false);

    pType analyze_type(pNode root);

    void generate_single(pNode root);
    void generate_global_var(Pointer<Ast::VarDefNode> root);
    void generate_function(Pointer<Ast::FuncDefNode> root);
    bool analyze_statement_node(pNode root);
    void copy_to_array(pNode root, Ir::pInstr addr, Pointer<ArrayType> t,
                       Pointer<Ast::ArrayDefNode> n);
    Ir::pVal analyze_opr(Pointer<Ast::BinaryNode> root);
    Ir::pVal cast_to_type(pNode root, Ir::pVal val, pType tr);

    Value from_array_def(Pointer<Ast::ArrayDefNode> n, Pointer<ArrayType> t);

    Ir::pModule module() const;

    bool current_block_end() const;
    Ir::pInstr add_instr(Ir::pInstr instr);

    EnvWrapper<MaybeConstInstr> _env;
    EnvWrapper<ImmValue> _const_env;

    void set_func(String sym, Ir::pFunc fun);
    bool func_count(String sym);
    Ir::pFunc find_func(String sym);

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

} // namespace AstToIr
