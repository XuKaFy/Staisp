#pragma once

#include "ir_block.h"
#include "ir_func.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(TypedSym var, Vector<pType> arg_types, Vector<String> arg_name);

    String print_func() const;

    void add_body(pInstr instr);
    // 将该块使用到的临时变量存放起来
    // 避免被释放
    // 例如临时数字或者 SymNode ，因为 Use 本身不是以 shared_ptr 指向 value
    void add_imm(pVal val);

    void end_function();

    Vector<String> arg_name;
    Vector<pVal> imms;
    Instrs args;
    Instrs body;
    Instrs allocs;

    BlockedProgram p;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(TypedSym var, Vector<pType> arg_types,
                               Vector<String> syms);

} // namespace Ir
