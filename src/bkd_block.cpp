#include "bkd_block.h"

namespace Backend {

String Block::print(std::string const& function_name) const
{
    String res = function_name + "_" + name + ":\n";
    
    for (auto &&i : body) {
        res += i->instr_print(function_name);
    }

    return res;
}

} // namespace Backend
