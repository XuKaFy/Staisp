#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

class LineGenerator
{
public:
    LineGenerator()
        : _line(0), _label_line(0) { }
    int line();
    Symbol label();

private:
    int _line;
    int _label_line;
};

}; // namespace Ir
