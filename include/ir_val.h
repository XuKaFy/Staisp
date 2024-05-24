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

    Vector<pUse> users;
    pType ty;

    void replace_self(Val *val);

    virtual ValType type() const = 0;

private:
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

    pUse operand(size_t index) const;
    size_t operand_size() const;

    Vector<pUse> operands;
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