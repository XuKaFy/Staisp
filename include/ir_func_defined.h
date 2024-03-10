#pragma once

#include "ir_func.h"
#include "ir_block.h"
#include "ir_sym.h"
#include "ir_mem_instr.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(TypedSym var, Vector<TypedSym> arg_types);

    virtual Symbol print_impl() const override;
    Symbol print_func() const;

    pBlock current_block() const;
    void add_block(pBlock block);
    pInstr add_instr(pInstr ir);

    Vector<pBlock> body;
    Vector<pInstr> args_value;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(TypedSym var, Vector<TypedSym> arg_types);

} // namespace ir
