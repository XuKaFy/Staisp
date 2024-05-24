#include "ir_func.h"

#include <memory>

#include <utility>

namespace Ir {

pFunctionType Func::functon_type() const { return to_function_type(ty); }

pFunc make_func(const TypedSym &var, Vector<pType> arg_types,
                bool variant_length) {
    return std::make_shared<Func>(var, std::move(arg_types), variant_length);
}

} // namespace Ir
