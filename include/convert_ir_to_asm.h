#pragma once

#include <utility>

#include "bkd_func.h"
#include "bkd_global.h"
#include "bkd_instr.h"
#include "bkd_module.h"
#include "ir_func_defined.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_module.h"

namespace Backend {

struct Convertor {
    Module convert(const Ir::pModule &mod);
    Global convert(const Ir::pGlobal &glob);
};

} // namespace BackendConvertor
