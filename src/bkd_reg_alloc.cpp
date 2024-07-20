#include <bkd_reg_alloc.h>
#include <bkd_func.h>
#include <type.h>

namespace Backend {

void Func::allocate_register() {
}

void Func::save_register() {
}

bool BlockValue::operator==(const BlockValue &b) const {
    return uses == b.uses;
}

bool BlockValue::operator!=(const BlockValue &b) const {
    return !operator==(b);
}

void BlockValue::cup(const BlockValue &v) {
    for (const auto &i : v.uses) {
        uses.insert(i);
    }
}

void BlockValue::clear() {
    uses.clear();
}

void TransferFunction::operator()(const Block *p, BlockValue &v) {
    // in = use + (in - def)
    auto use = p->use();
    auto def = p->def();

    for (auto i : def) {
        v.uses.erase(i);
    }
    for (auto i : use) {
        v.uses.insert(i);
    }
}

int TransferFunction::operator()(const Block *p, const BlockValue &IN, const BlockValue &OUT) {
    return 0;
}

int Func::liveness_analysis() {
    Map<const Block *, BlockValue> INs;
    Map<const Block *, BlockValue> OUTs;
    std::deque<const Block *> pending_blocks;
    TransferFunction transfer;
    int ans = 0;
    for (const auto &i : blocks) {
        INs[&i] = BlockValue();
        OUTs[&i] = BlockValue();
        pending_blocks.push_back(&i);
    }
    while (!pending_blocks.empty()) {
        const Block *b = *pending_blocks.begin();
        pending_blocks.pop_front();

        BlockValue old_IN = INs[b];
        BlockValue &IN = INs[b];
        BlockValue &OUT = OUTs[b];

        OUT.clear();
        auto out_block = b->out_blocks;
        if (!out_block.empty()) {
            OUT = INs[*out_block.begin()];
            for (auto block : out_block) {
                OUT.cup(INs[block]);
            }
        }

        IN = OUT;
        transfer(b, IN); // transfer function

        if (old_IN != IN) {
            auto in_block = b->in_blocks;
            pending_blocks.insert(pending_blocks.end(), in_block.begin(), in_block.end());
        }
    }
    for (const auto &i : blocks) {
        ans += transfer(&i, INs[&i], OUTs[&i]);
    }
    return ans;
}
}
