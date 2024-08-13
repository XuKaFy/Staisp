#include "ir_val.h"

#include <utility>

#include <memory>

namespace Ir {

void user_release_use(User *user, const pUse &i) {
    for (auto j = user->_operands.begin(); j != user->_operands.end();) {
        if (j->get() == i.get()) {
            j = user->_operands.erase(j);
        } else {
            ++j;
        }
    }
}

void val_release_use(Val *usee, const pUse &i) {
    for (auto j = usee->_users.begin(); j != usee->_users.end();) {
        if (j->get() == i.get()) {
            j = usee->_users.erase(j);
        } else {
            ++j;
        }
    }
}

bool Val::has_name() { return !_name.empty(); }

void Val::set_name(String name) { _name = std::move(name); }

void Val::replace_self(Val *val) {
    for (auto &i : _users) {
        // printf("replace %s(%llx) val %s with %s\n", i->user->name(), i->user,
        // i->usee->name(), val->name());
        i->usee = val;
        val->_users.push_back(i);
    }
    _users.clear();
}

String Val::name() const { return _name; }

void User::add_operand(Val *val) {
    pUse use = std::make_shared<Use>(this, val);
    // printf("OPERAND %llx, %s(%llx)\n", this, val->name(), val.get());
    _operands.push_back(use);
    val->_users.push_back(use);
}

void User::add_operand(const pVal &val) { add_operand(val.get()); }

void User::change_operand(size_t index, Val *val) {
    val_release_use(_operands[index]->usee, _operands[index]);
    _operands[index]->usee = val;
    val->_users.push_back(_operands[index]);
}

void User::release_operand(size_t index)
{
    val_release_use(_operands[index]->usee, _operands[index]);
    _operands.erase(_operands.begin() + index);
}

void User::change_operand(size_t index, const pVal &val) {
    change_operand(index, val.get());
}

size_t User::operand_size() const { return _operands.size(); }

pUse User::operand(size_t index) const { return _operands[index]; }

void val_release(Val *val) {
    for (const auto &i : val->_users) {
        auto user = i->user;
        user_release_use(user, i);
    }
    val->_users.clear();
}

void user_release(User *user) {
    for (const auto &i : user->_operands) {
        auto usee = i->usee;
        val_release_use(usee, i);
    }
    user->_operands.clear();
}

// add a pure use
// DONT use this except you know what you are doing
pUse Val::add_use(User* user) {
    pUse use = std::make_shared<Use>(user, this);
    _users.push_back(use);
    return use;
}

// remove a pure use
// DONT use this except you know what you are doing
bool Val::remove_use(pUse use) {
    for (auto i = _users.begin(); i != _users.end(); ++i) {
        if ((*i) == use) {
            _users.erase(i);
            return true;
        }
    }
    return false;
}

} // namespace Ir
