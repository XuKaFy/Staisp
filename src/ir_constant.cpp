#include "ir_constant.h"

#include <utility>

namespace Ir {

pVal make_constant(const Value& var) { return pVal(new Const(var)); }

} // namespace Ir
