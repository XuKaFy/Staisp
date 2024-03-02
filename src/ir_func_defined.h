#pragma once

#include "ir_func.h"
#include "ir_block.h"
#include "ir_sym.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(ImmType tr, pSym func_name, Vector<ImmType> arg_types)
        : Func(tr, func_name, arg_types) {
        pBlock first_block = make_block();
        size_t line_count = 0;
        for(auto i : arg_types) {
            pInstr tmp = first_block->add_instr(make_arg_instr(i));
            tmp->line = line_count++;
        }
        body.push_back(first_block);
    }

    virtual Symbol print_impl() const override;
    Symbol to_string() const;

    void add_block(pBlock block);

    Vector<pBlock> body;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(ImmType tr, pSym func_name, Vector<ImmType> arg_types);

} // namespace ir
