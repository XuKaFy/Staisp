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

void Block::add_imm(pVal imm)
{
    imms.push_back(imm);
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

void BlockedProgram::from_instrs(Instrs &instrs)
{
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
                break;
            }
            default:
                break;
            }
        }
        default:
            break;
        }
        push_back(std::move(i));
    }
    instrs.clear();

    generate_cfg();
}

void BlockedProgram::re_generate() const
{
    LineGenerator g;
    for(auto i : blocks) {
        g.generate(i->body);
    }
}

void BlockedProgram::generate_cfg() const
{
    Map<String, Block*> labelToBlock;

    for(auto i : blocks) {
        labelToBlock[i->label()->name()] = i.get();
        i->in_block.clear();
        i->out_block.clear();
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
            auto dest = static_cast<Instr*>(r->operand(0)->usee);
            // printf("finding %s\n", dest->name());
            i->connect(labelToBlock[dest->name()]);
            break;
        }
        case IR_BR_COND: {
            auto r = std::static_pointer_cast<BrCondInstr>(end);
            auto dest1 = static_cast<Instr*>(r->operand(1)->usee);
            auto dest2 = static_cast<Instr*>(r->operand(2)->usee);
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

void BlockedProgram::join_blocks()
{
    bool found = true;
    while(found) {
        found = false;
        for(auto i = blocks.begin(); i!=blocks.end(); ++i) {
            if((*i)->in_block.size() == 1 && (*i)->in_block[0]->out_block.size() == 1) {
                auto next_block = (*i)->in_block[0];
                next_block->body.pop_back();
                for(auto j = (*i)->body.begin()+1; j!=(*i)->body.end(); ++j) {
                    next_block->body.push_back(std::move(*j));
                }
                for(auto j : (*i)->imms) {
                    next_block->add_imm(j);
                }
                i = blocks.erase(i);
                found = true;
                generate_cfg();
                break;
            }
        }
    }
}

} // namespace ir
