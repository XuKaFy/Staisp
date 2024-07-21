#include "bkd_module.h"

namespace Backend {

String Module::print_module() const
{
    String res;

    res += ".text\n";
    res += ".global main\n";
    res += "\n";

    for (auto &&i : globs) {
        res += i.print();
    }

    for (auto &&func : funcs) {
        res += func.generate_asm();
    }

	if (res.find("call __builtin_fill_zero") != std::string::npos)
		res += R"(
.text
.globl  __builtin_fill_zero
__builtin_fill_zero:
    slli    a1,a1,2
    add     a1,a0,a1
__builtin_fill_zero_loop:
    addi    a0,a0,4
    sw      zero,-4(a0)
    bne     a1,a0,__builtin_fill_zero_loop
    ret
)";
    // -O3
    std::ignore = R"(
.text
.globl  __builtin_fill_zero
__builtin_fill_zero:
    slli    a2,a1,2
    li      a1,0
    tail    memset
)";

    return res;
}

} // namespace Backend
