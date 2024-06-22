#include "ir_line_generator.h"

namespace Ir {

void LineGenerator::generate(const Instrs &body) {
    for (const auto &i : body) {
        if (!i->has_name() || i->name()[0] == 'L' ||
            i->name()[0] == '%') { // re-generate all labels and reg name
            switch (i->ty->type_type()) {
            case TYPE_IR_TYPE: {
                switch (to_ir_type(i->ty)) {
                    case IR_LABEL:
                    i->set_name("L" + std::to_string(_label_line++));
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
                i->set_name("%" + std::to_string(_reg_line++));
                break;
            case TYPE_VOID_TYPE:
                break;
            }
        }
    }
}

}; // namespace Ir
