#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <filesystem>

#include "test_lib.hpp"

namespace fs = std::filesystem;

const bool interpret = false;

void judge(const std::string &id, const std::string& ll, const std::string &in, const std::string &out) {
    ASSERT_FALSE(
        system(("clang -DWANT_FILL_ZERO -S -emit-llvm ../../lib/sylib.c -o " + id + ".sylib.ll")
                   .c_str()));
    ASSERT_FALSE(system(("llvm-link -S " + ll + " " + id + ".sylib.ll -o " +
                         id + ".final.ll")
                            .c_str()));

    auto id_out = id + ".out";
    std::string io;
    if (fs::exists(in)) {
        io += " < ";
        io += in;
    }
    io += " > ";
    io += id_out;

    int code;
    if (interpret) {
        code = system(("lli " + id + ".final.ll" + io).c_str());
    } else {
        ASSERT_FALSE(system(("llc " + id + ".final.ll -o " + id + ".s").c_str()));
        ASSERT_FALSE(system(("gcc " + id + ".s -no-pie -o " + id).c_str()));
        code = system(("./" + id + io).c_str());
    }
    code = WEXITSTATUS(code);
    std::string actual = read(id_out);
    std::string expected = read(out);
    actual = combine(actual, code);

    // always remove
    remove((id + ".sylib.ll").c_str());

    ASSERT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));

    // remove if success
    remove((id + ".out").c_str());
    remove((id + ".sy.ll").c_str());
    if (!interpret)
        remove((id + ".s").c_str());
    remove((id + ".final.ll").c_str());
    remove((id).c_str());
}

// ulimit -s 16384
void test_sysy_frontend(const std::string &filename) {
    auto id = get_id(filename);
    auto path = get_path(filename);
    Stopwatch stopwatch;

    double t1 = stopwatch.timeit([&] {
        std::string format = "../frontend/SysYFrontend -S -O1";
        format = format + " -o " + path + ".sy.s";
        format = format + " -ll " + path + ".sy.ll";
        format = format + " " + path + ".sy";
        printf("%s\n", format.c_str());
        ASSERT_FALSE(system(format.c_str()));
    });

    double t2 = 0; // NO JUDGEMENT FOR FRONTEND NOW
    // double t2 = stopwatch.timeit([&] {
    //     judge(id, path + ".sy.ll", path + ".in", path + ".out");
    // });

    // if you hope to watch detailed time info, use following command:
    // ctest -R frontend -V | grep sysy_tests
    printf("\t| %-55s | %5.2fs | %5.2fs | \n", id.c_str(), t1, t2);
}
