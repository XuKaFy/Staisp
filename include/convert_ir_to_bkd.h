#pragma once

#include "bkd_module.h"
#include "ir_module.h"

namespace BackendConvertor {

class Convertor {
    Backend::pModule convert(const Ir::pModule &mod);
};

} // namespace BackendConvertor
