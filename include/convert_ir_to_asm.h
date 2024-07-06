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
    int excess_arguments = 0;
    int local_variables = 0;
    Ir::pFuncDefined func;
    Backend::Func bkd_func;
    int local_ordinal;
    explicit FunctionConvertor(Ir::pFuncDefined func)
        : func(std::move(func)), bkd_func(this->func->name()), local_ordinal(this->func->arg_name.size()) {}

    Backend::Func convert();
    Backend::Block convert(const Ir::pBlock &block);
    Backend::MachineInstrs convert(const Ir::pInstr &instr);

    Backend::Block generate_prolog();
    Backend::Block generate_epilog();

    int calculate_sp() const {
        int sp = local_variables + excess_arguments;
        if (sp % 8 != 0) sp += 8 - sp % 8;
        return sp;
    }
};

} // namespace BackendConvertor
