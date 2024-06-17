#include "ir_block.h"

#include "ir_control_instr.h"
#include "ir_func_defined.h"
#include "convert_ast_to_ir.h"
#include "ir_instr.h"
#include "ir_line_generator.h"

#include "ir_constant.h"

#include <memory>
#include <utility>

namespace Ir {

Pointer<LabelInstr> Block::label() const {
    return std::dynamic_pointer_cast<LabelInstr>(front());
}

void Block::push_behind_end(const pInstr &instr)
{
    instr->set_block(this);
    insert(std::prev(end()), instr);
}

void Block::push_after_label(const pInstr &instr)
{
    instr->set_block(this);
    insert(std::next(begin()), instr);
}

pVal Block::add_imm(Value value) {
    return program()->add_imm(std::move(value));
}

void Block::squeeze_out(bool selected) {
    auto end = back();
    auto new_br = std::dynamic_pointer_cast<Ir::BrCondInstr>(end)->select(!selected);
    // printf("Block {\n%s\n} selected %d\n\n", this->name().c_str(), selected);
    pop_back();
    push_back(new_br);
}

void Block::connect_in_and_out() {
    auto out_block = out_blocks();
    my_assert(out_block.size() == 1 && size() == 2, "?");
    for (auto j : in_blocks()) {
        j->replace_out(this, *out_block.begin());
    }
}

String Block::print_block() const {
    String whole_block;
    for (const auto &i : body) {
        // printf("    %s\n", i->instr_print().c_str());
        whole_block += i->instr_print();
        whole_block += "\n";
    }
    return whole_block;
}

pBlock make_block() {
    return std::make_shared<Block>();
}

void Block::replace_out(Block *before, Block *out) {
    auto bk = back();
    switch (bk->instr_type()) {
    case INSTR_BR: {
        bk->change_operand(0, out->label());
        break;
    }
    case INSTR_BR_COND: {
        bool flag = false;
        for (size_t i = 1; i <= 2; ++i) {
            if (bk->operand(i)->usee == before->label().get()) {
                bk->change_operand(i, out->label());
                flag = true;
            }
        }
        if (!flag) {
            printf("Warning: for block %s, try to replace %s wiith %s\n",
                   name().c_str(), before->name().c_str(), out->name().c_str());
            printf("Current Block %s: \n%s\n", name().c_str(),
                   print_block().c_str());
            throw Exception(2, "Block::replace_out", "not replaced");
        }
        break;
    }
    default:
        printf("Warning: block %s's last instruction is type %d\n",
               name().c_str(), bk->instr_type());
        throw Exception(1, "Block::replace_out", "not a br block");
    }
}

void BlockedProgram::initialize(Instrs instrs, Vector<pVal> args, ConstPool cpool) {
    LineGenerator g;
    g.generate(instrs);

    this->params_ = std::move(args);
    this->cpool = std::move(cpool);

    /* for(auto &&i : instrs) {
        printf(":: %s\n", i->instr_print());
    }
    puts("--end");*/

    for (const auto &i : instrs) {
        switch (i->ty->type_type()) {
        case TYPE_IR_TYPE: {
            switch (to_ir_type(i->ty)) {
            case IR_LABEL: {
                auto b = make_block();
                push_back(b);
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
        push_back(i);
    }
    instrs.clear();
}

pVal BlockedProgram::add_imm(Value value)
{
    return cpool.add(std::move(value));
}

void BlockedProgram::re_generate() const {
    LineGenerator g;
    for (const auto &i : blocks) {
        g.generate(i->body);
    }
}

Set<Block *> Block::in_blocks() const {
    Set<Block *> ans;
    for (auto &&i : label()->users) {
        auto user = static_cast<Instr *>(i->user);
        if (user->instr_type() != INSTR_BR && user->instr_type() != INSTR_BR_COND)
            continue;
        ans.insert(user->block());
    }
    return ans;
}

Set<Block *> Block::out_blocks() const {
    switch (back()->instr_type()) {
    case INSTR_BR:
        return {static_cast<LabelInstr *>(back()->operand(0)->usee)->block()};
    case INSTR_BR_COND:
        return {static_cast<LabelInstr *>(back()->operand(1)->usee)->block(),
                static_cast<LabelInstr *>(back()->operand(2)->usee)->block()};
    default:
        break;
    }
    return {};
}

} // namespace Ir
