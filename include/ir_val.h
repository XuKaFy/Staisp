#pragma once

#include "def.h"

#include "type.h"

namespace Ir {

struct Use;
typedef Pointer<Use> pUse;

struct Val;
struct User;

void val_release(Val* val);
void user_release(User* user);

enum ValType {
    VAL_CONST,
    VAL_GLOBAL,
    VAL_INSTR,
    VAL_BLOCK,
    VAL_FUNC,
};

struct Val {
    Val(pType ty)
        : ty(ty) { }

    virtual ~Val() {
        val_release(this);
    }

    bool has_name();
    void set_name(Symbol name);
    void set_name(String name);
    Symbol name() const;

    Vector<pUse> users;
    pType ty;

    void replace_self(Val* val);

    virtual ValType type() const = 0;

private:
    Symbol _name { nullptr };
};
typedef Pointer<Val> pVal;

struct User : public Val {
    User(pType ty)
        : Val(ty) { }

    virtual ~User() {
        user_release(this);
    }

    void add_operand(pVal val);
    void add_operand(Val* val);
    void change_operand(size_t index, Val* val);
    void change_operand(size_t index, pVal val);
    
    pUse operand(size_t index) const;
    size_t operand_size() const;

    Vector<pUse> operands;
};
typedef Pointer<User> pUser;

struct Use {
    Use(User* user, Val* val)
        : user(user), usee(val) { }

    // do NOT modify it except "ir_val.cpp"
    User* user;
    // do NOT modify it except "ir_val.cpp"
    Val* usee;
};

} // namespace ir