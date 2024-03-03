#pragma once

#include "ir_func.h"
#include "ir_block.h"
#include "ir_sym.h"
#include "ir_mem_instr.h"

namespace Ir {

struct FuncDefined : public Func {
    FuncDefined(ImmType tr, pSym func_name, Vector<TypedSym> arg_types)
        : Func(tr, func_name, arg_types) {
        pBlock first_block = make_block();
        first_block->add_instr(make_label_instr());
        
        size_t line_count = 1;
        for(auto i : arg_types) {
            pInstr tmp = first_block->add_instr(make_alloc_instr(i.tr));
            tmp->line = line_count++;
            first_block->add_instr(make_store_instr(i.tr, tmp, make_sym_instr(i)));
        }
        
        body.push_back(first_block);
    }

    virtual Symbol print_impl() const override;
    Symbol print_func() const;

    void add_block(pBlock block);

    Vector<pBlock> body;
};

typedef Pointer<FuncDefined> pFuncDefined;

pFuncDefined make_func_defined(ImmType tr, pSym func_name, Vector<TypedSym> arg_types);

} // namespace ir
