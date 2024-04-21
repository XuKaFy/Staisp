#pragma once

#include "def.h"

#include "type.h"

namespace Ir {

struct Use;
typedef Pointer<Use> pUse;

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
        users.clear();
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
        operands.clear();
    }

    void add_operand(pVal val);
    void add_operand(Val* val);
    pUse operand(size_t index) const;
    size_t operand_size() const;

private:
    Vector<pUse> operands;
};
typedef Pointer<User> pUser;

struct Use {
    Use(User* user, Val* val)
        : user(user), usee(val) { }

    User* user;
    Val* usee;
};

} // namespace ir