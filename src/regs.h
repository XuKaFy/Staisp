#pragma once

#include "def.h"

#include <cstring>

enum RegType
{
    REG_NULL,
    REG_NATIVE_32,
    REG_NATIVE_64,
    REG_STACK_32,
    REG_STACK_64,
    REG_VAL_32,
    REG_VAL_64,
};

struct Reg
{
    static inline Reg reg(RegType type, int id) {
        return Reg { type, id, {} };
    }

    static inline Reg var(RegType type, String name) {
        return Reg { type, {}, name};
    }

    static inline const Reg& null() {
        static Reg x = Reg { REG_NULL, {}, {} };
        return x;
    }

    void print() const;
    bool invaild() const;

    RegType type { REG_NULL };
    Integer id;
    String val_name;
};

class Regs
{
public:
    Regs();

    Reg allocate();
    void free(const Reg &x);
    Reg var(String name);

private:
    Reg x[31];
    bool used[31];
    Reg sp;
    Reg pc;
    Vector<Reg> stack;
    Map<String, Reg> vars;
};
