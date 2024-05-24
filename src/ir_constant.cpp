#include "ir_constant.h"

namespace Ir {

pVal make_constant(const Value &var) { return pVal(new Const(var)); }

} // namespace Ir
