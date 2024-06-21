#pragma once

#include <utility>

#include "def.h"
#include "imm.h"
#include "type.h"

#include "ir_val.h"

namespace Ir {

enum InstrType {
    INSTR_SYM,
    INSTR_LABEL,
    INSTR_BR,
    INSTR_BR_COND,
    INSTR_FUNC,
    INSTR_CALL,
    INSTR_RET,
    INSTR_CAST,
    INSTR_CMP,
    INSTR_STORE,
    INSTR_LOAD,
    INSTR_ALLOCA,
    INSTR_UNARY,
    INSTR_BINARY,
    INSTR_ITEM,
    INSTR_PHI,
    INSTR_UNREACHABLE,
};

struct Block;

struct Instr : public User {
    Instr(pType ty) : User(std::move(ty)) {}

    virtual String instr_print() const;

    ValType type() const override { return VAL_INSTR; }

    virtual InstrType instr_type() const { return INSTR_SYM; }

    bool is_terminator() const {
        switch (instr_type()) {
        case INSTR_RET:
        case INSTR_BR:
        case INSTR_BR_COND:
        case INSTR_UNREACHABLE:
            return true;
        default:
            break;
        }
        return false;
    }

    Block* block() {
        return block_;
    }

private:
    friend struct Block;

    void set_block(Block* block) {
        block_ = block;
    }

    Block *block_ {nullptr};
};

// 所有只要操作数为常量就可以计算出值的操作
struct CalculatableInstr : public Instr {
    CalculatableInstr(pType ty) : Instr(std::move(ty)) {}

    // 接受同等长度的常量并且计算出结果
    virtual ImmValue calculate(Vector<ImmValue> v) const = 0;
};

using pInstr = Pointer<Instr>;
using Instrs = List<pInstr>;

pInstr make_empty_instr();
pInstr make_sym_instr(const TypedSym &sym);

} // namespace Ir
