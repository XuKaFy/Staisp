#pragma once

#include "ir_block.h"
#include "ir_func.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(const TypedSym &var, Vector<pType> arg_types,
                Vector<String> arg_name);

    String print_func() const;

    void add_body(const pInstr &instr);
    // 将该块使用到的临时变量存放起来
    // 避免被释放
    // 例如临时数字或者 SymNode ，因为 Use 本身不是以 shared_ptr 指向 value
    void add_imm(const pVal &val);

    void end_function();

    Vector<String> arg_name;
    Vector<pVal> imms;
    Vector<pInstr> args;
    Instrs body;

    BlockedProgram p;
};

using pFuncDefined = Pointer<FuncDefined>;

pFuncDefined make_func_defined(const TypedSym &var, Vector<pType> arg_types,
                               Vector<String> syms);

} // namespace Ir
