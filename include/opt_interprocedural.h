#pragma once

#include "ir_val.h"
#include "opt.h"


namespace Optimize {

void inline_all_function(const Ir::pModule &mod, AstToIr::Convertor &convertor);

void global2local(const Ir::pModule &mod);

}