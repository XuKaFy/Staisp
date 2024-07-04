#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

class RegGenerator {
public:
    RegGenerator() = default;

    void generate(const Vector<pVal>& params);
    void generate(const Instrs &body);

private:
    int _reg_line{0};
    int _label_line{0};
};

}; // namespace Ir
