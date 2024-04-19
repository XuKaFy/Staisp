#include "opt.h"

namespace Optimize {

void optimize(Ir::pModule mod)
{
    for(auto i : mod->funsDefined) {
        Ir::BlockedProgram p;
        p.from_instrs(i->body);
        remove_empty_block(p);
        constant_propagate(p);
        i->body = p.re_generate();
    }
}

void remove_empty_block(Ir::BlockedProgram &p)
{
    my_assert(p.blocks.size(), "?");
    for(auto i = p.blocks.begin() + 1; i != p.blocks.end();) {
        if((*i)->in_block.size() == 0) {
            i = p.blocks.erase(i);
        } else ++i;
    }
}

void constant_propagate(Ir::BlockedProgram &p)
{
    ;
}

void reaching_definition_analyze(Ir::BlockedProgram &p)
{
    ;
}

void live_variable_analyze(Ir::BlockedProgram &p)
{
    ;
}

void available_expression_analyze(Ir::BlockedProgram &p)
{
    ;
}

} // namespace Optimize
