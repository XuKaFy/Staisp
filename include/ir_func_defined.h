#pragma once

#include "ir_func.h"
#include "ir_block.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> arg_name);

    Symbol print_func() const;

    void add_body(pInstr instr);
    void add_imm(pVal val);

    void end_function();

    Vector<Symbol> arg_name;
    Vector<pVal> imms;
    Instrs args;
    Instrs body;

    BlockedProgram p;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types, Vector<Symbol> syms);

} // namespace ir
