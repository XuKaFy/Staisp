#pragma once

#include "def.h"

#include "ast_node.h"
#include "ir_module.h"
#include "ir_opr_instr.h"

namespace AstToIr {

class Convertor {
public:
    Ir::pModule generate(Ast::AstProg asts);

private:
    void generate_single(Ast::pNode root, Ir::pModule mod);
    void generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod);
    void generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod);
    void analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func, Ir::pModule mod);
    Ir::pInstr analyze_value(Ast::pNode root, Ir::pFuncDefined func);

    Map<String, Ir::pInstr> var_map;
};


} // namespace ast_to_ir
