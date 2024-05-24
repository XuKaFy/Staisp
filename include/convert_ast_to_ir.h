// 将 AST 转化为 LLVM IR

#pragma once

#include "def.h"
#include "env.h"

#include "ast_node.h"
#include "ir_cmp_instr.h"
#include "ir_func_defined.h"
#include "ir_instr.h"
#include "ir_module.h"
#include "ir_opr_instr.h"
#include "type.h"

namespace AstToIr {

struct LoopEnv;
using pLoopEnv = Pointer<LoopEnv>;

struct LoopEnv {
    Ir::pInstr loop_begin;
    Ir::pInstr loop_end;
};

using LoopEnvStack = Stack<pLoopEnv>;

struct MaybeConstInstr {
    Ir::pInstr instr;
    bool is_const;
};

class Convertor {
public:
    Ir::pModule generate(const AstProg &asts);

    static Ir::BinInstrType fromBinaryOpr(const Pointer<Ast::BinaryNode> &root,
                                          const pType &ty);
    static Ir::CmpType fromCmpOpr(const Pointer<Ast::BinaryNode> &root,
                                  const pType &ty);

private:
    static void throw_error(const pNode &root, int id, const String &msg);

    Ir::pVal find_left_value(const pNode &root, const String &sym,
                             bool request_not_const = false);
    ImmValue find_const_value(const pNode &root, const String &sym);

    ImmValue constant_eval(const pNode &node);

    Ir::pVal analyze_value(const pNode &root, bool request_not_const = false);
    Ir::pVal analyze_left_value(const pNode &root,
                                bool request_not_const = false);

    pType analyze_type(const pNode &root);

    void generate_single(const pNode &root);
    void generate_global_var(const Pointer<Ast::VarDefNode> &root);
    void generate_function(const Pointer<Ast::FuncDefNode> &root);
    bool analyze_statement_node(const pNode &root);
    void copy_to_array(pNode root, Ir::pVal addr, const Pointer<ArrayType> &t,
                       const Pointer<Ast::ArrayDefNode> &n, bool constant);
    Ir::pVal analyze_opr(const Pointer<Ast::BinaryNode> &root);
    Ir::pVal cast_to_type(const pNode &root, Ir::pVal val, const pType &ty);

    Value from_array_def(const Pointer<Ast::ArrayDefNode> &n,
                         const Pointer<ArrayType> &t);

    Ir::pModule module() const;

    bool current_block_end() const;
    Ir::pInstr add_instr(Ir::pInstr instr);

    EnvWrapper<MaybeConstInstr> _env;
    EnvWrapper<ImmValue> _const_env;

    void set_func(const String &sym, Ir::pFunc fun);
    bool func_count(const String &sym);
    Ir::pFunc find_func(const String &sym);

    Map<String, Ir::pFunc> _func_map;

    pLoopEnv loop_env();
    void push_loop_env(Ir::pInstr begin, Ir::pInstr end);
    bool has_loop_env() const;
    void end_loop_env();
    void clear_loop_env();

    void add_initialization(const TypedSym &var,
                            const Pointer<Ast::ArrayDefNode> &node);

    LoopEnvStack _loop_env_stack;
    Ir::pModule _mod;
    Ir::pFuncDefined _cur_func;
    Ir::pFuncDefined _initializer_func;
    AstProg _prog;
};

} // namespace AstToIr
