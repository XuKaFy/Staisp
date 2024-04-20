#pragma once

#include "ir_block.h"
#include "ir_module.h"

#include "opt_1.h"

namespace Optimize {

void optimize(Ir::pModule mod);

void remove_empty_block(Ir::BlockedProgram &p);

} // namespace Optimize

