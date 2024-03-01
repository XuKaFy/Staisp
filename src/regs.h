#pragma once

#include "def.h"

#include <cstring>

struct Reg
{
    Immediate id;
    Symbol name;
};

typedef Pointer<Reg> pReg;

struct Regs {
    Regs();
    Reg x[32];
    Reg pc;
};
