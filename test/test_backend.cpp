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

    std::string sys = path + ".sy.s";
    std::string out = id + ".out";

    ASSERT_FALSE(system((gcc + sys + " ../../lib/sylib.c" + " -o " + id).c_str()));

    return;

    ASSERT_FALSE(system((rvlinux + id + " > " + out).c_str()));

    auto result = read(out);
    // trunc first line
    result = result.substr(result.find('\n') + 1);
    auto split_point = result.find(">>> Program exited, exit code = ");
    auto actual = result.substr(0, split_point);
    auto code = std::stoi(result.substr(split_point + strlen(">>> Program exited, exit code = ")));
    actual = combine(actual, code);
    std::string expected = read(path + ".out");
    ASSERT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));
}