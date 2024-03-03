#pragma once

#include "ir_func.h"
#include "ir_block.h"
#include "ir_sym.h"
#include "ir_mem_instr.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(TypedSym var, Vector<TypedSym> arg_types)
        : Func(var, arg_types) {
        pBlock first_block = make_block();
        
        for(auto i : arg_types) {
            pInstr tmp = first_block->add_instr(make_alloc_instr(i.tr));
            first_block->add_instr(make_store_instr(i.tr, tmp, make_sym_instr(i)));
        }
        
        body.push_back(first_block);
    }

    virtual Symbol print_impl() const override;
    Symbol print_func() const;

    pBlock current_block() const;
    void add_block(pBlock block);
    pInstr add_instr(pInstr ir);

    Vector<pBlock> body;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(TypedSym var, Vector<TypedSym> arg_types);

} // namespace ir
