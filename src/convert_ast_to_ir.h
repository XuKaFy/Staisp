#pragma once

#include "def.h"

#include "ast_node.h"
#include "ir_module.h"
#include "ir_opr_instr.h"
#include "ir_cmp_instr.h"
#include "ir_call_instr.h"

namespace AstToIr {

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

    Map<String, Ir::pInstr> var_map;
    Map<String, Ir::pFunc> func_map;
};


} // namespace ast_to_ir
