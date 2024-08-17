#pragma once

#include "ir_instr.h"
#include "ir_control_instr.h"
#include "ir_block.h"

namespace Ir {

struct PhiInstr final : Instr {
    PhiInstr(const pType &type);

    InstrType instr_type() const override { return INSTR_PHI; }

    String instr_print() const override;

    void add_incoming(LabelInstr *blk, Val *val);
    void remove(LabelInstr *label);

    Ir::LabelInstr *phi_label(size_t x) const {
        return static_cast<Ir::LabelInstr*>(operand(x * 2 + 1)->usee);
    }

    void change_phi_label(size_t x, LabelInstr* l) {
        change_operand(x * 2 + 1, l);
    }

    Ir::Val *phi_val(size_t x) const {
        return static_cast<Ir::Block*>(operand(x * 2)->usee);    
    }

    size_t phi_pairs() const {
        return operand_size() / 2;
    }

    struct iterator {
        const PhiInstr* phi; size_t i;

        std::pair<LabelInstr*, Val*> operator*() const {
            return {phi->phi_label(i), phi->phi_val(i)};
        }

        iterator& operator++() {
            ++i; return *this;
        }
        iterator& operator++(int) = delete;
        bool operator==(iterator const& other) const {
            return phi == other.phi && i == other.i;
        }
        bool operator!=(iterator const& other) const {
            return !operator==(other);
        }
    };

    iterator begin() const {
        return {this, 0};
    }

    iterator end() const {
        return {this, phi_pairs()};
    }

    Instr* clone_internal() const override {
        auto res = new PhiInstr(ty);
        for (auto [label, val] : *this) {
            res->add_incoming(label, val);
        }
        return res;
    }
};

// make_phi_instr: create a new PhiInstr with the given type
[[nodiscard]] std::shared_ptr<PhiInstr> make_phi_instr(const pType &type);
}; // namespace Ir
