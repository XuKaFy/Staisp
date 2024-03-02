#pragma once

#include "def.h"

#include "ast_node.h"
#include "ir_module.h"

namespace AstToIr {

class Convertor {
public:
    Ir::pModule generate(Ast::AstProg asts);

private:
    void generate_single(Ast::pNode root, Ir::pModule mod);
    void generate_global_var(Pointer<Ast::VarDefNode> root, Ir::pModule mod);
    void generate_function(Pointer<Ast::FuncDefNode> root, Ir::pModule mod);
    void analyze_statement_node(Ast::pNode root, Ir::pFuncDefined func);

    Map<String, int> var_map;
};


} // namespace ast_to_ir
