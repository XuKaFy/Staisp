#include "ir_reg_generator.h"

namespace Ir {

void RegGenerator::generate(const Vector<pVal>& params) {
    for (auto&& param : params) {
        param->set_name("%" + std::to_string(_reg_line++));
    }
}

void RegGenerator::generate(const Instrs &body) {
    for (const auto &instr : body) {
        if (!instr->has_name() || instr->name()[0] == 'L' ||
            instr->name()[0] == '%') { // re-generate all labels and reg name
            switch (instr->ty->type_type()) {
            case TYPE_IR_TYPE: {
                switch (to_ir_type(instr->ty)) {
                    case IR_LABEL:
                    instr->set_name("L" + std::to_string(_label_line++));
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
                instr->set_name("%" + std::to_string(_reg_line++));
                break;
            case TYPE_VOID_TYPE:
                break;
            }
        }
    }
}

}; // namespace Ir
