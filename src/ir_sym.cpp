#include "ir_sym.h"

namespace Ir {

Symbol Sym::print_impl() const
{
    return to_symbol(String("@") + name);
}

pSym make_sym(Symbol name)
{
    return pSym(new Sym(name));
}

} // namespace ir
