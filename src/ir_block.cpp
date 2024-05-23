#include "ir_block.h"

#include "ir_control_instr.h"
#include "ir_instr.h"
#include "ir_line_generator.h"

#include "ir_constant.h"
#include "ir_opr_instr.h"

#include <memory>

namespace Ir {

Pointer<LabelInstr> Block::label() const { return std::dynamic_pointer_cast<LabelInstr>(body.front()); }

pInstr Block::back() const { return body.back(); }

void Block::add_imm(pVal imm) { imms.push_back(imm); }

void Block::squeeze_out(bool selected) {
    auto end = body.back();
    auto new_br = std::dynamic_pointer_cast<Ir::BrCondInstr>(end)->select(1 - selected);
    new_br->block = this;
    body.pop_back();
    body.push_back(new_br);
}

void Block::connect_in_and_out() {
    auto out_block = out_blocks();
    my_assert(out_block.size() == 1 && body.size() == 2, "?");
    for (auto j : in_blocks()) {
        j->replace_out(this, *out_block.begin());
    }
}

String Block::print_block() const {
    String whole_block;
    for (size_t i = 0; i < body.size(); ++i) {
        whole_block += body[i]->instr_print();
        whole_block += "\n";
    }
    return whole_block;
}

void Block::push_back(pInstr instr) { body.push_back(instr); }

pBlock make_block() { return pBlock(new Block()); }

void Block::replace_out(Block *before, Block *out) {
    auto back = body.back();
    switch (back->instr_type()) {
    case INSTR_BR: {
        back->change_operand(0, out->label());
        break;
    }
    case INSTR_BR_COND: {
        bool flag = false;
        for (size_t i = 1; i <= 2; ++i) {
            if (back->operand(i)->usee == before->label().get()) {
                back->change_operand(i, out->label());
                flag = true;
            }
        }
        if (!flag) {
            throw Exception(2, "Block::replace_out", "not replaced");
        }
        break;
    }
    default:
        throw Exception(1, "Block::replace_out", "not a br block");
    }
}

void BlockedProgram::push_back(pInstr instr) {
    blocks.back()->push_back(instr);
}

void BlockedProgram::from_instrs(Instrs &instrs) {
    LineGenerator g;
    g.generate(instrs);

    /* for(auto &&i : instrs) {
        printf(":: %s\n", i->instr_print());
    }
    puts("--end");*/

    for (auto i : instrs) {
        switch (i->ty->type_type()) {
        case TYPE_IR_TYPE: {
            switch (to_ir_type(i->ty)) {
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
        i->block = blocks.back().get();
        push_back(std::move(i));
    }
    instrs.clear();
}

void BlockedProgram::re_generate() const {
    LineGenerator g;
    for (auto i : blocks) {
        g.generate(i->body);
    }
}

Vector<Block*> Block::in_blocks() const
{
    Vector<Block*> ans;
    for(auto &&i : label()->users) {
        auto user = static_cast<Instr*>(i->user);
        ans.push_back(user->block);
    }
    return ans;
}

Vector<Block*> Block::out_blocks() const
{
    switch(back()->instr_type()) {
    case INSTR_BR:
        return { static_cast<LabelInstr *>(back()->operand(0)->usee)->block };
    case INSTR_BR_COND:
        return { static_cast<LabelInstr *>(back()->operand(1)->usee)->block,
                 static_cast<LabelInstr *>(back()->operand(2)->usee)->block };
    default: break;
    }
    return { };
}

void BlockedProgram::opt_join_blocks() {
    for (auto i = blocks.begin(); i != blocks.end(); ) {
        /*
        \     |     /
         [next_block]
              |
         [cur_block]
          /   |   \
        */
        auto cur_block = i->get();
        auto in_blocks = cur_block->in_blocks();
        if (in_blocks.size() != 1) {
            ++i;
            continue;
        }

        auto next_block = *in_blocks.begin();
        if (next_block->out_blocks().size() != 1) {
            ++i;
            continue;
        }
    
        // printf("Will remove %s\n", cur_block->name());
        // printf("Block {\n%s} connected with {\n%s}\n", next_block->print_block().c_str(), cur_block->print_block().c_str());
            
        next_block->body.pop_back();
        for (auto j = cur_block->body.begin() + 1;
             j != cur_block->body.end(); ++j) {
            (*j)->block = next_block;
            next_block->body.push_back(std::move(*j));
        }
        for (auto j : cur_block->imms) {
            next_block->add_imm(j);
        }
    
        i = blocks.erase(i);
    }
}

void BlockedProgram::opt_remove_empty_block() {
    my_assert(blocks.size(), "?");
    for (auto i = blocks.begin() + 1; i != blocks.end();) {
        if ((*i)->in_blocks().size() == 0) {
            // printf("Block {\n%s} removed\n", (*i)->print_block().c_str());
            i = blocks.erase(i);
        } else
            ++i;
    }
}

void BlockedProgram::opt_connect_empty_block() {
    my_assert(blocks.size(), "?");
    for (auto i = blocks.begin() + 1; i != blocks.end();) {
        if ((*i)->out_blocks().size() == 1 && (*i)->body.size() == 2) {
            // printf("Remove Empty Br Block %s\n", (*i)->name());
            // printf("Block {\n%s} connected\n", (*i)->print_block().c_str());
            (*i)->connect_in_and_out();
            i = blocks.erase(i);
        } else
            ++i;
    }
}

bool can_be_removed(Ir::InstrType t) {
    switch (t) {
    case Ir::INSTR_RET:
    case Ir::INSTR_BR:
    case Ir::INSTR_BR_COND:
    case Ir::INSTR_STORE:
    case Ir::INSTR_CALL:
    case Ir::INSTR_UNREACHABLE:
        return false;
    default:
        return true;
    }
    return true;
}

void BlockedProgram::opt_remove_dead_code() {
    my_assert(blocks.size(), "?");
    for (auto i : blocks) {
        for (auto j = i->body.begin() + 1; j != i->body.end();) {
            if ((*j)->users.empty() && can_be_removed((*j)->instr_type())) {
                j = i->body.erase(j);
            } else {
                ++j;
            }
        }
    }

    for (auto i : blocks) {
        if (i->body.size() <= 1)
            continue;

        auto end = i->body.back();
        if (end->instr_type() != Ir::INSTR_BR_COND)
            continue;

        auto cond = end->operand(0)->usee;
        if (cond->type() != Ir::VAL_CONST)
            continue;

        auto con = static_cast<Ir::Const *>(cond);
        if (con->v.type() == VALUE_IMM) {
            i->squeeze_out((bool)con->v);
        }
    }
}

void BlockedProgram::normal_opt() {
    opt_remove_empty_block();
    opt_connect_empty_block();
    opt_join_blocks();
    opt_remove_dead_code();
}


void BlockedProgram::opt_trivial() {
    // this is not correct
    for (auto block : blocks) {
        for (auto instr : block->body) {
            if (auto bin = std::dynamic_pointer_cast<Ir::BinInstr>(instr)) {
                auto usee =  bin->operand(1)->usee;
                if (bin->binType == INSTR_SDIV && usee->type() == VAL_CONST) {
                    auto value = static_cast<Const*>(usee)->v;
                    if (value.type() == ValueType::VALUE_IMM && value.imm_value().ty == IMM_I32 && value.imm_value().val.ival == 2) {
                        bin->binType = INSTR_ASHR;
                        auto imm = Ir::make_constant(ImmValue(1));
                        block->add_imm(imm);
                        bin->operand(1)->usee->replace_self(imm.get());
                    }
                }
            }
        }
    }
}

} // namespace Ir
