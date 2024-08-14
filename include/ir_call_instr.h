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

    Instr* clone_internal() const override {
        Vector<Val*> v;
        for (auto &&i = std::next(operands().begin()); i != operands().end(); ++i) {
            v.push_back((*i)->usee);
        }
        return new CallInstr(dynamic_cast<Func*>(operands().front()->usee), v);
    }

    pFunctionType func_ty;
};

struct RetInstr : public Instr {
    RetInstr(Val *oprd = nullptr) : Instr(make_ir_type(IR_RET)) {
        if (oprd) {
            add_operand(oprd);
        }
    }

    Instr* clone_internal() const override {
        if (operands().empty())
            return new RetInstr();
        return new RetInstr(operands()[0]->usee);
    }

    InstrType instr_type() const override { return INSTR_RET; }

    String instr_print() const override;
};

struct UnreachableInstr : Instr {
    UnreachableInstr() : Instr(make_ir_type(IR_UNREACHABLE)) {}

    InstrType instr_type() const override { return INSTR_UNREACHABLE; }

    Instr* clone_internal() const override {
        return new UnreachableInstr;
    }

    String instr_print() const override { return "unreachable"; }
};

pInstr make_call_instr(Func *func, const Vector<Val*> &args);
pInstr make_ret_instr(Val *oprd = nullptr);
pInstr make_unreachable_instr();

} // namespace Ir
