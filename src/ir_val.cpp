#include "ir_val.h"

namespace Ir {

Symbol Val::print()
{
    if(str_form == nullptr) {
        return str_form = print_impl();
    }
    return str_form;
}

} // namespace ir
