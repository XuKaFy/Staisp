#include "bkd_ir_instr.h"
#include "def.h"
#include <string>

namespace Backend {

std::string nameOf(Reg reg) {
    return std::string(REG_NAME[(int) reg]);
}

} // namespace Backend
