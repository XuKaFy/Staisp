#include "ir_val.h"

namespace Ir {

bool Val::has_name()
{
    return _name;
}

void Val::set_name(Symbol name)
{
    _name = name;
}

void Val::set_name(String name)
{
    // printf("NAME SET %s(%llx)\n", to_symbol(name), this);
    set_name(to_symbol(name));
}

void Val::replace_self(Val* val)
{
    for(auto i : users) {
        // printf("replace %s(%llx) val %s with %s\n", i->user->name(), i->user, i->usee->name(), val->name());
        i->usee = val;
    }
}

Symbol Val::name() const
{
    return _name;
}

void User::add_operand(pVal val)
{
    pUse use = pUse(new Use { this, val.get() });
    // printf("OPERAND %llx, %s(%llx)\n", this, val->name(), val.get());
    operands.push_back(use);
    val->users.push_back(use);
}

size_t User::operand_size() const
{
    return operands.size();
}

pUse User::operand(size_t index) const
{
    return operands[index];
}

} // namespace ir
