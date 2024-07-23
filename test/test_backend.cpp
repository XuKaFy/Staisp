#include <gtest/gtest.h>
#include <string>

#include "test_lib.hpp"

#include <filesystem>
namespace fs = std::filesystem;

// requirement:
// sudo apt install gcc-12-riscv64-linux-gnu qemu-user qemu-system-misc
void test_sysy_backend(const std::string &filename) {
    auto id = get_id(filename);
    auto path = get_path(filename);
    const std::string gcc = "riscv64-linux-gnu-gcc-12 -static ";
    const std::string qemu = "qemu-riscv64 ";

    std::string sys = path + ".sy.s";
    std::string in = path + ".in";
    std::string id_out = id + ".out";

    ASSERT_FALSE(system((gcc + sys + " ../../lib/sylib.c" + " -o " + id).c_str()));

    std::string io = qemu + id;
    if (fs::exists(in)) {
        io += " < ";
        io += in;
    }
    io += " > " + id_out;
    int code = system(io.c_str());
    code = WEXITSTATUS(code);
    auto actual = read(id_out);
    auto expected = read(path + ".out");
    actual = combine(actual, code);
    ASSERT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));
}