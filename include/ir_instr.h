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
#ifdef USING_MINI_GEP
    INSTR_MINI_GEP,
#endif
    INSTR_PHI,
    INSTR_UNREACHABLE,
};

struct Block;
struct Instr;
using pInstr = Pointer<Instr>;

struct CloneContext {
public:
    Ir::Val* lookup(Ir::Val* v) const {
        if (auto i = map_.find(v); i != map_.end()) {
            return i->second;
        }
        return v;
    }
    void map(const Ir::Val* f, Ir::Val* s) {
        map_[f] = s;
    }
private:
    Map<const Ir::Val*, Ir::Val*> map_;
};

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

    Instr* clone(CloneContext &ctx) const {
        auto ret = clone_internal();
        ctx.map(this, ret);
        return ret;
    }

    void fix_clone(CloneContext &ctx) {
        Vector<Val*> v;
        for (auto i : operands()) {
            v.push_back(ctx.lookup(i->usee));
        }
        release_all_operands();
        for (auto &&i : v) {
            add_operand(i);
        }
    }

protected:
    virtual Instr* clone_internal() const {
        return new Instr(ty);
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

using Instrs = List<pInstr>;

pInstr make_empty_instr();
pInstr make_sym_instr(const TypedSym &sym);

} // namespace Ir
