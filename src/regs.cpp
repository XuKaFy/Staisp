#include "regs.h"

Regs::Regs()
{
    memset(used, false, sizeof(used));
    for(int i=0; i<31; ++i) {
        x[i] = Reg::reg(REG_NATIVE_64, i);
    }
    sp = Reg::reg(REG_NATIVE_64, 31);
    pc = Reg::reg(REG_NATIVE_64, 32);
}

Reg Regs::allocate()
{
    for(int i=0; i<31; ++i) {
        if(used[i]) continue;
        used[i] = true;
        return x[i];
    }
    stack.push_back(Reg::reg(REG_STACK_64, stack.size()));
    return stack.back();
}

void Regs::free(const Reg &x)
{
    switch(x.type) {
    case REG_NATIVE_32:
    case REG_NATIVE_64:
        used[x.id] = false;
        return ;
    case REG_STACK_32:
    case REG_STACK_64:
        stack.pop_back();
        return ;
    default:
        // cannot happen
        return ;
    }
}

Reg Regs::var(String name)
{
    if(vars.count(name)) {
        return vars[name];
    }
    return Reg::null();
}

void Reg::print() const
{
    switch(type) {
    case REG_NATIVE_32:
    case REG_NATIVE_64:
        printf("%%x%02d ", id);
        return ;
    case REG_STACK_32:
    case REG_STACK_64:
        printf("[sp+%d] ", id);
        return ;
    default:
        // cannot happen
        return ;
    }
}

bool Reg::invaild() const
{
    return type == REG_NULL;
}
