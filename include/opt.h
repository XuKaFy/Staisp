#pragma once

#include "ir_block.h"
#include "ir_module.h"

namespace Optimize {

void optimize(Ir::pModule mod);

void remove_empty_block(Ir::BlockedProgram &p);
void remove_dead_code(Ir::BlockedProgram &p);

} // namespace Optimize

