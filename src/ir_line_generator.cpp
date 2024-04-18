#include "ir_line_generator.h"

namespace Ir {

int LineGenerator::line()
{
    return _line++;
}

Symbol LineGenerator::label()
{
    return to_symbol(std::to_string(++_label_line));
}

};
