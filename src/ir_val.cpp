#include "ir_val.h"

#include <utility>

#include <memory>

namespace Ir {

void user_release_use(User *user, const pUse &i) {
    for (auto j = user->operands.begin(); j != user->operands.end();) {
        if (*j == i) {
            j = user->operands.erase(j);
        } else {
            ++j;
        }
    }
}

void val_release_use(Val *usee, const pUse &i) {
    for (auto j = usee->users.begin(); j != usee->users.end();) {
        if (*j == i) {
            j = usee->users.erase(j);
        } else {
            ++j;
        }
    }
}

bool Val::has_name() { return !_name.empty(); }

void Val::set_name(String name) { _name = std::move(name); }

void Val::replace_self(Val *val) {
    for (auto &i : users) {
        // printf("replace %s(%llx) val %s with %s\n", i->user->name(), i->user,
        // i->usee->name(), val->name());
        i->usee = val;
        val->users.push_back(i);
    }
    users.clear();
}

String Val::name() const { return _name; }

void User::add_operand(Val *val) {
    pUse use = std::make_shared<Use>(this, val);
    // printf("OPERAND %llx, %s(%llx)\n", this, val->name(), val.get());
    operands.push_back(use);
    val->users.push_back(use);
}

void User::add_operand(const pVal &val) { add_operand(val.get()); }

void User::change_operand(size_t index, Val *val) {
    val_release_use(operands[index]->usee, operands[index]);
    operands[index]->usee = val;
    val->users.push_back(operands[index]);
}

void User::change_operand(size_t index, const pVal &val) {
    change_operand(index, val.get());
}

size_t User::operand_size() const { return operands.size(); }

pUse User::operand(size_t index) const { return operands[index]; }

void val_release(Val *val) {
    for (const auto &i : val->users) {
        auto *user = i->user;
        user_release_use(user, i);
    }
    val->users.clear();
}

void user_release(User *user) {
    for (const auto &i : user->operands) {
        auto *usee = i->usee;
        val_release_use(usee, i);
    }
    user->operands.clear();
}

} // namespace Ir
