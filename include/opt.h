#pragma once

#include "ir_block.h"
#include "ir_module.h"

namespace Optimize {

void optimize(Ir::pModule mod);

void remove_empty_block(Ir::BlockedProgram &p);
void constant_propagate(Ir::BlockedProgram &p);

void reaching_definition_analyze(Ir::BlockedProgram &p);
void live_variable_analyze(Ir::BlockedProgram &p);
void available_expression_analyze(Ir::BlockedProgram &p);

} // namespace Optimize

