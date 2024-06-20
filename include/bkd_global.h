#pragma once

#include "bkd_ir_instr.h"
#include "def.h"
#include "value.h"

namespace Backend {

enum class GlobalPartType {
    ZERO, // note that zero 1 is 1 byte
    WORD, // note that a word is 4 byte
};

struct GlobalPart {
    String print() const;

    GlobalPartType ty;
    int val;
};

struct Global {
    Global(String name, int val);
    Global(String name, ArrayValue val);

    String print() const;

    pLabelInstr label;
    Vector<GlobalPart> component;
};

} // namespace Backend
