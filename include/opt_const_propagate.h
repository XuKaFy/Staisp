#pragma once

#include "def.h"
#include "imm.h"
#include "ir_block.h"
#include "ir_constant.h"
#include "ir_instr.h"
#include "type.h"
#include <algorithm>
#include <functional>
#include <optional>

namespace OptConstPropagate {

// #define OPT_CONST_PROPAGATE_DEBUG

typedef std::optional<ImmValue> Val;

struct ConstantMap {
    Val value(Ir::Instr *instr) const {
        if (last_found.has_value() && last_found.value() != val.end() &&
            last_found.value()->first == instr) {
            return last_found.value()->second;
        }
        last_found = val.find(instr);
        if (last_found == val.end()) {
            printf("key %s not found\n", instr->instr_print().c_str());
            throw Exception(1, "ConstantMap", "Error: no key");
        }
        return last_found.value()->second;
    }

    bool hasValue(Ir::Instr *instr) const {
        last_found = val.find(instr);
        return last_found == val.end() ? false : true;
    }

    bool isValueNac(Ir::Instr *instr) const {
        Val val = value(instr);
        return val.has_value() == false;
    }

    void setValue(Ir::Instr *instr, Val v) {
        if (hasValue(instr) && value(instr).has_value()) {
            if (v.has_value()) {
                if (value(instr).value() != v.value()) {
                    setValueNac(instr);
                }
            } else {
                setValueNac(instr);
            }
        } else {
#ifdef OPT_CONST_PROPAGATE_DEBUG
            if (v.has_value()) {
                printf("    SET [%s] = %s\n", instr->instr_print().c_str(),
                       v->print().c_str());
            } else {
                printf("    CANCEL FROM SET [%s]\n",
                       instr->instr_print().c_str());
            }
#endif
            val[instr] = v;
            last_found = std::nullopt;
        }
    }

    void setValueNac(Ir::Instr *instr) {
        val[instr] = std::nullopt;
#ifdef OPT_CONST_PROPAGATE_DEBUG
        printf("    CANCEL [%s], has_value = %d\n",
               instr->instr_print().c_str(), val[instr].has_value());
#endif
    }

    void erase(Ir::Instr *instr) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
        printf("    ERASE [%s]\n", instr->instr_print().c_str());
#endif
        val.erase(instr);
        last_found = std::nullopt;
    }

    void transfer(Ir::Instr *to, Ir::Instr *from) {
#ifdef OPT_CONST_PROPAGATE_DEBUG
        printf("    TRANSFER [%s] from [%s]\n", to->instr_print().c_str(),
               from->instr_print().c_str());
#endif
        setValue(to, value(from));
    }

    void
    ergodic(std::function<void(Ir::Instr *instr, ImmValue val)> func) const {
        for (auto&& i : val) {
            if (i.second.has_value()) {
                func(i.first, i.second.value());
            }
        }
    }

    void cup(const ConstantMap &map);

    void clear() { val.clear(); }

    bool operator==(const ConstantMap &map) const { return val == map.val; }

    void saveArrayValue(Ir::Instr* arr, int64_t index, const ImmValue &v) {
        array_val[arr][index] = v;
    }

    std::optional<ImmValue> getArrayValue(Ir::Instr* arr, int64_t index) const {
        if (!array_val.count(arr))
            return {};
        auto found = array_val.at(arr);
        return found[index];
    }

    void clearArrayValue(Ir::Instr* arr) {
        array_val.erase(arr);
    }

    void eraseArrayValueLeq(Ir::Instr* arr, int64_t index) {
        for (auto i = array_val[arr].begin(); i != array_val[arr].end(); ) {
            if (i->first < index) {
                i = array_val[arr].erase(i);
            } else {
                ++i;
            }
        }
    }

private:
    Map<Ir::Instr*, Val> val;
    Map<Ir::Instr*, Map<int64_t, ImmValue>> array_val;
    mutable std::optional<Map<Ir::Instr *, Val>::const_iterator> last_found;
};

struct BlockValue {
    bool operator==(const BlockValue &b) const { return val == b.val; }
    bool operator!=(const BlockValue &b) const { return !operator==(b); }

    void cup(const BlockValue &v) {
        val.cup(v.val);
    }

    void clear() { val.clear(); }

    ConstantMap val;
};

struct TransferFunction {
    void operator()(Ir::Block *p, BlockValue &v);
    int operator()(Ir::Block *p, const BlockValue &IN, const BlockValue &OUT);
};

} // namespace OptConstPropagate
