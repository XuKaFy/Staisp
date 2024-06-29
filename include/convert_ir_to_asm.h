#pragma once

#include "bkd_func.h"
#include "bkd_global.h"
#include "bkd_ir_instr.h"
#include "bkd_module.h"
#include "ir_func_defined.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_module.h"

namespace IrToAsm {

class Convertor {
public:
    Backend::pModule convert(const Ir::pModule &mod);
    Backend::Global convert(const Ir::pGlobal &glob);
    Backend::Func convert(const Ir::pFuncDefined &func);
    Backend::Block convert(const Ir::pFuncDefined &func, const Ir::pBlock &block);
    Backend::MachineInstrs convert(const Ir::pFuncDefined &func, const Ir::pInstr &instr);

    Backend::MachineInstrs generate_prolog();
    Backend::MachineInstrs generate_epilog();
};

} // namespace BackendConvertor
