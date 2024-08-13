#pragma once

#include "ir_func.h"
#include "ir_instr.h"
#include <iterator>

namespace Ir {

struct CallInstr : public Instr {
    CallInstr(Func* func, const Vector<Val*> &args)
        : Instr(to_function_type(func->ty)->ret_type),
          func_ty(to_function_type(func->ty)) {
        add_operand(func);
        for (const auto &i : args) {
            add_operand(i);
        }
    }

    InstrType instr_type() const override { return INSTR_CALL; }

    String instr_print() const override;

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new CallInstr(dynamic_cast<Func*>(new_operands.front()), 
            Vector<Val*>(std::next(new_operands.begin()), new_operands.end()));
    }

    pFunctionType func_ty;
};

struct RetInstr : public Instr {
    RetInstr(Val *oprd = nullptr) : Instr(make_ir_type(IR_RET)) {
        if (oprd) {
            add_operand(oprd);
        }
    }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        if (new_operands.empty())
            return new RetInstr();
        return new RetInstr(new_operands[0]);
    }

    InstrType instr_type() const override { return INSTR_RET; }

    String instr_print() const override;
};

struct UnreachableInstr : Instr {
    UnreachableInstr() : Instr(make_ir_type(IR_UNREACHABLE)) {}

    InstrType instr_type() const override { return INSTR_UNREACHABLE; }

    Instr* clone_internal(const Vector<Val*> new_operands) const override {
        return new UnreachableInstr;
    }

    String instr_print() const override { return "unreachable"; }
};

pInstr make_call_instr(Func *func, const Vector<Val*> &args);
pInstr make_ret_instr(Val *oprd = nullptr);
pInstr make_unreachable_instr();

} // namespace Ir
