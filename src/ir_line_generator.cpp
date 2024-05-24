#include "ir_line_generator.h"

namespace Ir {

int LineGenerator::line() { return _line++; }

String LineGenerator::label() { return std::to_string(++_label_line); }

void LineGenerator::generate(const Instrs &body) {
    for (const auto & i : body) {
        if (!i->has_name() || i->name()[0] == 'L' ||
            i->name()[0] == '%') { // re-generate all labels and reg name
            switch (i->ty->type_type()) {
            case TYPE_IR_TYPE: {
                switch (to_ir_type(i->ty)) {
                case IR_LABEL:
                    i->set_name("L" + label());
                case IR_BR:
                case IR_BR_COND:
                case IR_STORE:
                case IR_RET:
                case IR_UNREACHABLE:
                    break; // no need to alloc an id
                }
                break;
            }
            case TYPE_BASIC_TYPE:
            case TYPE_COMPOUND_TYPE:
                i->set_name("%" + std::to_string(line()));
                break;
            case TYPE_VOID_TYPE:
                // throw Exception(1, "LineGenerator::generate", "void
                // instruction");
                break;
            }
        }
        // printf("%s\n", body[i]->instr_print());
    }
    // puts("--");
}

}; // namespace Ir
