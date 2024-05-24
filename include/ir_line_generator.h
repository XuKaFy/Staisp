#pragma once

#include "def.h"

#include "ir_instr.h"

namespace Ir {

// 为若干段 Instrs 标注行号的类
// LineGenerator::generate 接受 instrs
// 并且为所有寄存器和 label 按顺序标号
class LineGenerator {
public:
    LineGenerator()  = default;
    int line();
    String label();

    void generate(const Instrs &body);

private:
    int _line{0};
    int _label_line{0};
};

}; // namespace Ir
