#include "bkd_global.h"
#include "imm.h"
#include "type.h"
#include <string>

namespace Backend {

Global::Global(String name, int val)
    : label(std::make_shared<LabelInstr>(name)), component({{ GlobalPartType::WORD, val}}) {
}

static void generate_array(Vector<GlobalPart> &component, ArrayValue v)
{
    pType elem_type = to_elem_type(v.ty);
    size_t elem_len = elem_type->length();
    size_t elem_cnt = v.values.size();
    size_t array_len = to_array_type(v.ty)->length();
    if (is_basic_type(elem_type)) {
        int load_cnt = 0;
        int should_load_cnt = 4 / elem_len;
        int val = 0;
        for (auto i : v.values) {
            ImmValue v = i->imm_value();
            val = (val << (32 / should_load_cnt)) + v.val.ival % ((1 << (32 / should_load_cnt)));
            if (++load_cnt == should_load_cnt) {
                load_cnt = 0;
                component.push_back(GlobalPart { GlobalPartType::WORD, val });
            }
        }
        if (load_cnt) {
            component.push_back(GlobalPart { GlobalPartType::WORD, val });
        }
        size_t loaded = elem_len * (elem_cnt % 4 ? elem_cnt + 4 - (elem_cnt % 4) : elem_cnt);
        if (loaded < array_len) {
            component.push_back(GlobalPart { GlobalPartType::ZERO, int(array_len - loaded) });
        }
        return ;
    }
    for (auto i : v.values) {
        generate_array(component, i->array_value());
    }
}

Global::Global(String name, ArrayValue v)
    : label(std::make_shared<LabelInstr>(name)) {
    generate_array(component, v);
}

String GlobalPart::print() const
{
    String res;
    switch (ty) {
    case GlobalPartType::ZERO:
        res =  "zero " + std::to_string(val);
        break;
    case GlobalPartType::WORD:
        res =  "word " + std::to_string(val);
        break;
    }
    return res;
}

String Global::print() const
{
    String res = label->instr_print() + "\n";
    for (auto &&i : component) {
        res += i.print() + "\n";
    }
    return res;
}

} // namespace Backend
