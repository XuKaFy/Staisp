#include <gtest/gtest.h>
#include <string>

#include "test_lib.hpp"

// requirement:
// 1. sudo apt install gcc-12-riscv64-linux-gnu
// 2. put binary or link of rvlinux at ~/riscv/rvlinux
void test_sysy_backend(const std::string &filename) {
    auto id = get_id(filename);
    auto path = get_path(filename);
    const std::string gcc = "riscv64-linux-gnu-gcc-12 -static ";
    const std::string rvlinux = "~/riscv/rvlinux ";

    std::string input = path + ".sy.s";

    ASSERT_FALSE(system((gcc + input + " -o " + id).c_str()));

    ASSERT_FALSE(system((rvlinux + id).c_str()));
}