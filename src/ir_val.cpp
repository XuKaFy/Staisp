#include "ir_val.h"

namespace Ir {

void user_release_use(User *user, pUse i)
{
    for(auto j = user->operands.begin(); j!=user->operands.end(); ) {
        if(*j == i) {
            j = user->operands.erase(j);
        } else {
            ++j;
        }
    }
}

void val_release_use(Val *usee, pUse i)
{
    for(auto j = usee->users.begin(); j!=usee->users.end(); ) {
        if(*j == i) {
            j = usee->users.erase(j);
        } else {
            ++j;
        }
    }
}

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
    for(auto &i : users) {
        // printf("replace %s(%llx) val %s with %s\n", i->user->name(), i->user, i->usee->name(), val->name());
        i->usee = val;
        val->users.push_back(i);
    }
    users.clear();
}

Symbol Val::name() const
{
    return _name;
}

void User::add_operand(Val* val)
{
    pUse use = pUse(new Use { this, val });
    // printf("OPERAND %llx, %s(%llx)\n", this, val->name(), val.get());
    operands.push_back(use);
    val->users.push_back(use);
}

void User::add_operand(pVal val)
{
    add_operand(val.get());
}

void User::change_operand(size_t index, Val* val)
{
    val_release_use(operands[index]->usee, operands[index]);
    operands[index]->usee = val;
    val->users.push_back(operands[index]);
}

void User::change_operand(size_t index, pVal val)
{
    change_operand(index, val.get());
}

size_t User::operand_size() const
{
    return operands.size();
}

pUse User::operand(size_t index) const
{
    return operands[index];
}

void val_release(Val* val)
{
    for(auto i : val->users) {
        auto user = i->user;
        user_release_use(user, i);
    }
    val->users.clear();
}

void user_release(User* user)
{
    for(auto i : user->operands) {
        auto usee = i->usee;
        val_release_use(usee, i);
    }
    user->operands.clear();
}

} // namespace ir
