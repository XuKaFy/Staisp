#include "ir_block.h"

namespace Ir {

pInstr Block::label() const
{
    return body.front();
}

pInstr Block::back() const
{
    return body.back();
}

Symbol Block::print_block() const
{
    String whole_block;
    for(size_t i=0; i<body.size(); ++i) {
        whole_block += body[i]->instr_print();
        whole_block += "\n";
    }
    return to_symbol(whole_block);
}

void Block::connect(Block* next)
{
    next->in_block.push_back(this);
    out_block.push_back(next);
}

void Block::push_back(pInstr instr)
{
    body.push_back(instr);
}

pBlock make_block()
{
    return pBlock(new Block());
}

void BlockedProgram::push_back(pInstr instr)
{
    blocks.back()->push_back(instr);
}

void BlockedProgram::from_instrs(const Instrs &instrs)
{
    Map<String, Block*> labelToBlock;
    LineGenerator g;
    g.generate(instrs);

    for(auto i : instrs) {
        switch(i->ty->type_type()) {
        case TYPE_IR_TYPE: {
            switch(to_ir_type(i->ty)) {
            case IR_LABEL: {
                auto b = make_block();
                blocks.push_back(b);
                b->set_name(i->name());
                labelToBlock[i->name()] = b.get();
                // printf("saving %s\n", i->name());
                break;
            }
            default:
                break;
            }
        }
        default:
            break;
        }
        push_back(i);
    }

    for(auto i : blocks) {
        auto end = i->back();
        my_assert(end->ty->type_type() == TYPE_IR_TYPE, "?");
        switch(to_ir_type(end->ty)) {
        case IR_RET: {
            auto r = std::static_pointer_cast<RetInstr>(end);
            break;
        }
        case IR_BR: {
            auto r = std::static_pointer_cast<BrInstr>(end);
            auto dest = std::static_pointer_cast<Instr>(r->operand(0)->usee);
            // printf("finding %s\n", dest->name());
            i->connect(labelToBlock[dest->name()]);
            break;
        }
        case IR_BR_COND: {
            auto r = std::static_pointer_cast<BrCondInstr>(end);
            auto dest1 = std::static_pointer_cast<Instr>(r->operand(1)->usee);
            auto dest2 = std::static_pointer_cast<Instr>(r->operand(2)->usee);
            // printf("finding %s\n", dest1->name());
            i->connect(labelToBlock[dest1->name()]);
            // printf("finding %s\n", dest2->name());
            i->connect(labelToBlock[dest2->name()]);
            break;
        }
        case IR_STORE:
            throw Exception(1, "BlockedProgram::from_instrs",
                "end is a store");
            break;
        case IR_LABEL:
            throw Exception(2, "BlockedProgram::from_instrs",
                 "end is a label or a empty block");
            break;
        }
    }
}

Instrs BlockedProgram::re_generate() const
{
    Instrs instrs;
    for(auto i : blocks) {
        for(auto j : i->body) {
            instrs.push_back(j);
        }
    }
    LineGenerator g;
    g.generate(instrs);
    return instrs;
}

} // namespace ir
