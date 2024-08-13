#pragma once

#include <utility>

#include "def.h"

#include "type.h"

namespace Ir {

struct Use;
using pUse = Pointer<Use>;

struct Val;
struct User;

void val_release(Val *val);
void user_release(User *user);
void val_release_use(Val *usee, const pUse &i);
void user_release_use(User *user, const pUse &i);

enum ValType {
    VAL_CONST,
    VAL_GLOBAL,
    VAL_INSTR,
    VAL_BLOCK,
    VAL_FUNC,
};

struct Val {
    Val(pType ty) : ty(std::move(std::move(ty))) {}

    virtual ~Val() { val_release(this); }

    bool has_name();
    void set_name(String name);
    String name() const;

    const Vector<pUse>& users() const { return _users; }

    pType ty;

    void replace_self(Val *val);

    virtual ValType type() const = 0;

    // add a pure use
    // DONT use this except you know what you are doing
    pUse add_use(User* user);

    // remove a pure use
    // DONT use this except you know what you are doing
    bool remove_use(pUse use);

private:
    friend struct User;
    friend void val_release(Val *val);
    friend void user_release(User *user);
    friend void val_release_use(Val *usee, const pUse &i);
    friend void user_release_use(User *user, const pUse &i);

    Vector<pUse> _users;
    String _name;
};
using pVal = Pointer<Val>;

struct User : public Val {
    User(pType ty) : Val(std::move(ty)) {}

    ~User() override { user_release(this); }

    void add_operand(const pVal &val);
    void add_operand(Val *val);
    void change_operand(size_t index, Val *val);
    void change_operand(size_t index, const pVal &val);
    void release_operand(size_t index);

    const Vector<pUse>& operands() const { return _operands; }

    pUse operand(size_t index) const;
    size_t operand_size() const;

private:
    friend void val_release(Val *val);
    friend void user_release(User *user);
    friend void val_release_use(Val *usee, const pUse &i);
    friend void user_release_use(User *user, const pUse &i);
    
    Vector<pUse> _operands;
};
using pUser = Pointer<User>;

struct Use {
    Use(User *user, Val *val) : user(user), usee(val) {}

    // do NOT modify it except "ir_val.cpp"
    User *user;
    // do NOT modify it except "ir_val.cpp"
    Val *usee;
};

} // namespace Ir