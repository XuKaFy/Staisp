#pragma once

#include <utility>

#include "bkd_func.h"
#include "bkd_global.h"
#include "bkd_ir_instr.h"
#include "bkd_module.h"
#include "ir_func_defined.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_module.h"

namespace IrToAsm {

struct Convertor {
    Backend::pModule convert(const Ir::pModule &mod);
    Backend::Global convert(const Ir::pGlobal &glob);
};

struct FunctionConvertor {
    int allocate_register = 32;
    Ir::pFuncDefined func;
    Backend::Func bkd_func;
    explicit FunctionConvertor(Ir::pFuncDefined func): func(std::move(func)), bkd_func(this->func->name()) {}

    Backend::Func convert();
    Backend::Block convert(const Ir::pBlock &block);
    Backend::MachineInstrs convert(const Ir::pInstr &instr);

    Backend::MachineInstrs generate_prolog();
    Backend::MachineInstrs generate_epilog();
};

} // namespace BackendConvertor
