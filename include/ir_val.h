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

    bool has_name();
    void set_name(Symbol name);
    void set_name(String name);
    Symbol name() const;

    Vector<pUse> users;
    pType ty;

    virtual ValType type() const = 0;

private:
    Symbol _name { nullptr };
};
typedef Pointer<Val> pVal;

struct User : public Val {
    User(pType ty)
        : Val(ty) { }

    void add_operand(pVal val);
    pUse operand(size_t index) const;
    size_t operand_size() const;

private:
    Vector<pUse> operands;
};
typedef Pointer<User> pUser;

struct Use {
    Use(pUser user, pVal val)
        : user(user), usee(val) { }

    pUser user;
    pVal usee;
};

} // namespace ir