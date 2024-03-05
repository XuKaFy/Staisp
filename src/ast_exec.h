#pragma once

#include "ast_node.h"

namespace Ast {

struct ConstexprException { };

Immediate execute(pNode root);

} // namespace ast
