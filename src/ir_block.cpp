#include "ir_block.h"

#include "alys_dom.h"
#include "def.h"
#include "ir_control_instr.h"
#include "ir_func_defined.h"
#include "convert_ast_to_ir.h"
#include "ir_phi_instr.h"
#include "ir_instr.h"
#include "ir_reg_generator.h"

#include "ir_constant.h"
#include "type.h"

#include <memory>
#include <utility>

namespace Ir {

void Block::erase_from_phi() {
    for (auto& use : label()->users()) {
        if (!use) {
            printf("Warning [erase from phi] Label %s has empty use\n", label()->name().c_str());
            continue;
        }
        auto phi = dynamic_cast<PhiInstr*>(use->user);
        if (phi) {
            // printf("BLOCK RELEASE WITH PHI [%s]\n", phi->instr_print().c_str());
            phi->remove(label().get());
        }
    }
}

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

pBlock Block::clone(CloneContext &context) const
{
    pBlock new_block = make_block();
    for (auto &&i = cbegin(); i != cend(); ++i) {
        new_block->push_back(pInstr((*i)->clone(context)));
    }
    return new_block;
}

void Block::fix_clone(CloneContext &context) const
{
    for (auto &&i = cbegin(); i != cend(); ++i) {
        (*i)->fix_clone(context);
    }
}

bool BlockedProgram::check_invalid_phi(String state)
{
    for (auto i : *this) {
        for (auto j : *i) {
            auto phi = dynamic_cast<PhiInstr*>(j.get());
            if (!phi) continue;
            if (phi->operand_size() % 2) {
                printf("IN STATE %s\n", state.c_str());
                printf("    INVALID PHI in %s\n", j->name().c_str());
                for (size_t i=0; i<phi->operand_size(); ++i) {
                    printf("    arg: %s\n", phi->operand(i)->usee->name().c_str());
                }
                return true;
            }
            Set<LabelInstr*> lbs;
            for (auto j : i->in_blocks()) {
                lbs.insert(j->label().get());
            }
            for (size_t i=0; i<phi->phi_pairs(); ++i) {
                lbs.erase(phi->phi_label(i));
            }
            if (!lbs.empty()) {
                printf("IN STATE %s\n", state.c_str());
                printf("    WRONG PHI in %s\n", j->name().c_str());
                for (size_t i=0; i<phi->operand_size(); ++i) {
                    printf("    arg: %s\n", phi->operand(i)->usee->name().c_str());
                }
                return true;
            }
        }
    }
    return false;
}

bool BlockedProgram::check_empty_use(String state)
{
    for (auto i : *this) {
        for (auto j : *i) {
            for (auto k : j->users()) {
                if(!k) {
                    printf("IN STATE %s\n", state.c_str());
                    printf("    EMPTY USE CHECKED in %s\n", j->name().c_str());
                    return true;
                }
            }
        }
    }
    return false;
}

void Block::squeeze_out(bool selected) {
    auto end = back();
    auto origin_br = std::dynamic_pointer_cast<Ir::BrCondInstr>(end);
    auto label_will_remove = dynamic_cast<LabelInstr*>(origin_br->operand(static_cast<int>(selected) + 1)->usee);
    // because of phi will remove itself from the use-def chain
    // so label()->users will be modified
    // we SHOULD copy users
    for (auto i : label()->users()) {
        auto phi = dynamic_cast<Ir::PhiInstr*>(i->user);
        if (phi && phi->block() == label_will_remove->block()) {
            // printf("SELECT BRANCH WITH PHI [%s]\n", phi->instr_print().c_str());
            // printf("  remove %s\n", label()->name().c_str());
            phi->remove(label().get());
        }
    }
    auto new_br = origin_br->select(!selected);
    // printf("Block {\n%s\n} selected %d\n\n", this->name().c_str(), selected);
    pop_back();
    push_back(new_br);
}

void Block::replace_in(Block* before, Block* in) {
    for (auto&& instr : *this) {
        auto phi = dynamic_cast<PhiInstr*>(instr.get());
        if (!phi)
            continue;
        for (size_t i=0; i<phi->phi_pairs(); ++i) {
            if (phi->phi_label(i)->block() == before) {
                phi->change_phi_label(i, in->label().get());
                break;
            }
        }
    }
}

void Block::connect_in_and_out() {
    auto out_block = *out_blocks().begin();

    Set<LabelInstr*> labels;

    for (auto block : in_blocks()) {
        labels.insert(block->label().get());
        block->replace_out(this, out_block);
    }

    for (auto&& instr : *out_block) {
        auto phi = dynamic_cast<PhiInstr*>(instr.get());
        if (!phi)
            continue;
            
        Val* branch_value = nullptr;
        
        for (size_t i=0; i<phi->phi_pairs(); ++i) {
            if (phi->phi_label(i) == label().get()) {
                // printf("CONNECT WITH PHI: [%s]\n", phi->instr_print().c_str());
                // printf("  %s -> %s\n", label()->name().c_str(), (*labels.begin())->name().c_str());
                phi->change_phi_label(i, *labels.begin());
                branch_value = phi->phi_val(i);
                // printf("    AFTER: [%s]\n", phi->instr_print().c_str());
                break;
            }
        }
        
        for (auto j = std::next(labels.begin()); j != labels.end(); ++j) {
            // printf("CONNECT ADDED WITH PHI: [%s]\n", phi->instr_print().c_str());
            phi->add_incoming(*j, branch_value);
            // printf("    AFTER: [%s]\n", phi->instr_print().c_str());
        }
    }
}

String Block::print_block() const {
    String whole_block;
    assert(!body.empty());
    for (const auto &i : body) {
        whole_block += i->instr_print();
        whole_block += "\n    ";
    }
    whole_block.resize(whole_block.size() - 4);
    return whole_block;
}

pBlock make_block() {
    return std::make_shared<Block>();
}

void Block::replace_out(Block *before, Block *out) {
    auto bk = back();
    switch (bk->instr_type()) {
    case INSTR_BR: {
        bk->change_operand(0, out->label().get());
        break;
    }
    case INSTR_BR_COND: {
        bool flag = false;
        for (size_t i = 1; i <= 2; ++i) {
            if (bk->operand(i)->usee == before->label().get()) {
                bk->change_operand(i, out->label().get());
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

bool BlockedProgram::has_calls() const
{
    for (auto i = cbegin(); i != cend(); ++i) {
        for (auto instr : *(*i)) {
            if (instr->instr_type() == INSTR_CALL)
                return true;
        }
    }
    return false;
}

void BlockedProgram::initialize(String name, Instrs instrs, Vector<pVal> args, ConstPool cpool) {
    name_ = name;
    
    RegGenerator g;
    g.generate(args);
    g.generate(instrs);

    this->params_ = std::move(args);
    this->cpool = std::move(cpool);

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
    RegGenerator g;
    g.generate(params());
    for (const auto &block : blocks) {
        g.generate(block->body);
    }
}

Set<Block *> Block::in_blocks() const {
    Set<Block *> ans;
    for (auto &&i : label()->users()) {
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

BlockedProgram BlockedProgram::clone() const
{
    BlockedProgram p;
    CloneContext ctx;
    p.cpool = cpool;
    p.name_ = name_;
    for (auto i : params_) {
        auto new_param = make_sym_instr(TypedSym("NOT_NAMED", i->ty));
        p.add_param(new_param);
        ctx.map(i.get(), new_param.get());
    }
    for (auto i = cbegin(); i != cend(); ++i) {
        p.push_back((*i)->clone(ctx));
    }
    for (auto i = p.cbegin(); i != p.cend(); ++i) {
        (*i)->fix_clone(ctx);
    }
    p.re_generate();
    return p;
}

} // namespace Ir
