#include "bkd_global.h"
#include "imm.h"
#include "type.h"
#include <string>

namespace Backend {

Global::Global(String name, int val)
    : name(std::move(name)), component({{ GlobalPartType::WORD, val}}) {
}

void generate_array(Vector<GlobalPart> &component, const ArrayValue& array) {
    pType elem_type = to_elem_type(array.ty);
    size_t elem_len = elem_type->length();
    size_t elem_cnt = array.values.size();
    size_t array_len = to_array_type(array.ty)->length();
    if (is_basic_type(elem_type)) {
        int load_cnt = 0;
        int should_load_cnt = 4 / elem_len; // assume that no 64bit
        int val = 0;
        for (auto&& i : array.values) {
            ImmValue v = i->imm_value();
            val = (val << (32 / should_load_cnt)) + v.val.ival % ((1ll << (32 / should_load_cnt)));
            if (++load_cnt == should_load_cnt) {
                component.push_back(GlobalPart { GlobalPartType::WORD, val });
                load_cnt = 0;
                val = 0;
            }
        }
        if (load_cnt) {
            component.push_back(GlobalPart { GlobalPartType::WORD, val });
        }
        size_t loaded = elem_len * (elem_cnt % should_load_cnt ? elem_cnt + should_load_cnt - (elem_cnt % should_load_cnt) : elem_cnt);
        if (loaded < array_len) {
            component.push_back(GlobalPart { GlobalPartType::ZERO, int(array_len - loaded) });
        }
        return ;
    }
    for (auto&& i : array.values) {
        generate_array(component, i->array_value());
    }
    size_t loaded = elem_len * elem_cnt;
    if (loaded < array_len) {
        component.push_back(GlobalPart { GlobalPartType::ZERO, int(array_len - loaded) });
    }
}

Global::Global(String name, const ArrayValue &array)
    : name(std::move(name)) {
    generate_array(component, array);
}

String GlobalPart::print() const
{
    String res;
    switch (ty) {
    case GlobalPartType::ZERO:
        res =  "    .zero " + std::to_string(val);
        break;
    case GlobalPartType::WORD:
        res =  "    .word " + std::to_string(val);
        break;
    }
    return res;
}

String Global::print() const
{
    String res;
    res += ".globl " + name + "\n";
    res += ".align 5\n"; // 32 bytes
    res += name + ":\n";
    for (auto &&i : component) {
        res += i.print() + "\n";
    }
    return res;
}

} // namespace Backend
