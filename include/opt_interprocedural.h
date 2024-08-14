#pragma once

#include "ir_val.h"
#include "opt.h"


namespace Optimize {

bool inline_all_function(const Ir::pModule &mod);

void global2local(const Ir::pModule &mod);

}