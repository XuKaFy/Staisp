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
    set_name(to_symbol(name));
}

Symbol Val::name() const
{
    return _name;
}

void User::add_operand(pVal val)
{
    pUse use = pUse(new Use { pUser(this), val });
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
