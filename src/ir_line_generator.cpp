#include "ir_line_generator.h"

namespace Ir {

int LineGenerator::line() { return _line++; }

Symbol LineGenerator::label() {
    return to_symbol(std::to_string(++_label_line));
}

void LineGenerator::generate(const Instrs &body) {
    for (size_t i = 0; i < body.size(); ++i) {
        if (!body[i]->has_name() || body[i]->name()[0] == 'L' ||
            body[i]->name()[0] == '%') { // re-generate all labels and reg name
            switch (body[i]->ty->type_type()) {
            case TYPE_IR_TYPE: {
                switch (to_ir_type(body[i]->ty)) {
                case IR_LABEL:
                    body[i]->set_name(String("L") + label());
                case IR_BR:
                case IR_BR_COND:
                case IR_STORE:
                case IR_RET:
                    break; // no need to alloc an id
                }
                break;
            }
            case TYPE_BASIC_TYPE:
            case TYPE_COMPOUND_TYPE:
                body[i]->set_name(String("%") + std::to_string(line()));
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
