#pragma once

#include "ir_module.h"
#include "convert_ast_to_ir.h"

namespace Optimize {

void optimize(const Ir::pModule &mod, AstToIr::Convertor &convertor);

} // namespace Optimize
