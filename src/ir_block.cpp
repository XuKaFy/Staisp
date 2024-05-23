#include "ir_block.h"

#include "ir_call_instr.h"
#include "ir_control_instr.h"
#include "ir_line_generator.h"

#include "ir_constant.h"
#include "ir_opr_instr.h"

namespace Ir {

pInstr Block::label() const { return body.front(); }

pInstr Block::back() const { return body.back(); }

void Block::add_imm(pVal imm) { imms.push_back(imm); }

void Block::squeeze_out(bool selected) {
    auto end = body.back();
    // 0->trueTo
    // 1->falseTo
    auto will_remove = end->operand(selected + 1)->usee;
    // printf("%s\n", std::static_pointer_cast<Ir::BrCondInstr>(end)->instr_print().c_str());
    auto new_br =
        std::static_pointer_cast<Ir::BrCondInstr>(end)->select(1 - selected);
    for (auto i : out_block) {
        if (i->label().get() == will_remove) {
            i->in_block.erase(this);
            out_block.erase(i);
            break;
        }
    }
    body.pop_back();
    body.push_back(new_br);
    // printf("    changed to %s\n", new_br->instr_print().c_str());
}

void Block::connect_in_and_out() {
    my_assert(out_block.size() == 1 && body.size() == 2, "?");
    auto out = *out_block.begin();
    for (auto j : in_block) {
        j->replace_out(this, out);
        out->in_block.insert(j);
    }
    out->in_block.erase(this);
}

String Block::print_block() const {
    String whole_block;
    for (size_t i = 0; i < body.size(); ++i) {
        whole_block += body[i]->instr_print();
        whole_block += "\n";
    }
    return whole_block;
}

void Block::connect(Block *next) {
    next->in_block.insert(this);
    out_block.insert(next);
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
    out_block.erase(before);
    out_block.insert(out);
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
        push_back(std::move(i));
    }
    instrs.clear();

    generate_cfg();
}

void BlockedProgram::re_generate() const {
    LineGenerator g;
    for (auto i : blocks) {
        g.generate(i->body);
    }
}

void BlockedProgram::generate_cfg() const {
    Map<String, Block *> labelToBlock;

    for (auto i : blocks) {
        labelToBlock[i->label()->name()] = i.get();
        i->in_block.clear();
        i->out_block.clear();
    }

    for (auto i : blocks) {
        auto end = i->back();
        my_assert(end->ty->type_type() == TYPE_IR_TYPE, "?");
        switch (to_ir_type(end->ty)) {
        case IR_RET: {
            auto r = std::static_pointer_cast<RetInstr>(end);
            break;
        }
        case IR_BR: {
            auto r = std::static_pointer_cast<BrInstr>(end);
            auto dest = static_cast<Instr *>(r->operand(0)->usee);
            // printf("finding %s\n", dest->name());
            i->connect(labelToBlock[dest->name()]);
            break;
        }
        case IR_BR_COND: {
            auto r = std::static_pointer_cast<BrCondInstr>(end);
            auto dest1 = static_cast<Instr *>(r->operand(1)->usee);
            auto dest2 = static_cast<Instr *>(r->operand(2)->usee);
            // printf("finding %s\n", dest1->name());
            i->connect(labelToBlock[dest1->name()]);
            // printf("finding %s\n", dest2->name());
            i->connect(labelToBlock[dest2->name()]);
            break;
        }
        case IR_STORE:
            throw Exception(1, "BlockedProgram::from_instrs", "end is a store");
            break;
        case IR_LABEL:
            throw Exception(2, "BlockedProgram::from_instrs",
                            "end is a label or a empty block");
            break;
        }
    }
}

void BlockedProgram::print_cfg() const {
    for (auto i : blocks) {
        printf("CFG: Block %s\n", i->name().c_str());
        for (auto j : i->in_block) {
            printf("    In Block %s\n", j->name().c_str());
        }
        for (auto j : i->out_block) {
            printf("    Out Block %s\n", j->name().c_str());
        }
    }
}

void BlockedProgram::opt_join_blocks() {
    bool found = true;
    while (found) {
        found = false;
        for (auto i = blocks.begin(); i != blocks.end(); ++i) {
            /*
            \     |     /
             [next_block]
                  |
             [cur_block]
              /   |   \
            */
            auto cur_block = i->get();
            if (cur_block->in_block.size() != 1)
                continue;

            auto next_block = *cur_block->in_block.begin();
            if (next_block->out_block.size() != 1)
                continue;

            // printf("Will remove %s\n", cur_block->name());

            next_block->body.pop_back();
            for (auto j = cur_block->body.begin() + 1;
                 j != cur_block->body.end(); ++j) {
                next_block->body.push_back(std::move(*j));
            }
            for (auto j : cur_block->imms) {
                next_block->add_imm(j);
            }

            next_block->out_block.clear();
            for (auto j : cur_block->out_block) {
                j->in_block.erase(cur_block);
                j->in_block.insert(next_block);
                next_block->out_block.insert(j);
            }
            i = blocks.erase(i);
            found = true;
            break;
        }
    }
}

void BlockedProgram::opt_remove_empty_block() {
    my_assert(blocks.size(), "?");
    for (auto i = blocks.begin() + 1; i != blocks.end();) {
        if ((*i)->in_block.size() == 0) {
            i = blocks.erase(i);
        } else
            ++i;
    }
}

void BlockedProgram::opt_connect_empty_block() {
    my_assert(blocks.size(), "?");
    for (auto i = blocks.begin() + 1; i != blocks.end();) {
        if ((*i)->out_block.size() == 1 && (*i)->body.size() == 2) {
            // printf("Remove Empty Br Block %s\n", (*i)->name());
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
