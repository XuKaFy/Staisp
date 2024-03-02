#pragma once

#include "def.h"

#include "ast_node.h"
#include "ir_module.h"

namespace AstToIr {

Ir::pModule generate(Ast::AstProg asts);

} // namespace ast_to_ir
