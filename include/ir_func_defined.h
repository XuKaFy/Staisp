#pragma once

#include "ir_func.h"
#include "ir_block.h"
#include "ir_mem_instr.h"

#include "ir_line_generator.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> arg_name);

    Symbol print_func() const;

    void add_body(pInstr instr);

    Vector<Symbol> arg_name;
    Instrs args;
    Instrs body;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> syms);

} // namespace ir
