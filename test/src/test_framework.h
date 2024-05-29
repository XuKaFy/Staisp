#pragma once

#include "def.h"

#include "gtest/gtest.h"
#include <fstream>

#include <cstdlib>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

String read(const String &file) {
    std::ifstream in;
    in.open(file, std::fstream::in);
    EXPECT_TRUE(in.is_open()) << file;

    String code((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    return code;
}

std::string normalizeLineEndings(const std::string &str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\r') {
            if (i + 1 < str.size() && str[i + 1] == '\n') {
                ++i; // Skip '\n'
            }
            while (!result.empty() && (isspace(result.back()) != 0)) {
                result.pop_back();
            }
            result += '\n';
        } else {
            if (str[i] == '\n') {
                while (!result.empty() && (isspace(result.back()) != 0)) {
                    result.pop_back();
                }
            }
            result += str[i];
        }
    }
    while (!result.empty() &&
           (result.back() == '\n' || (isspace(result.back()) != 0))) {
        result.pop_back();
    }
    return result;
}

const bool interpret = false;

void judge(const String &id, const String& ll, const String &in, const String &out) {
    ASSERT_FALSE(
        system(("clang -S -emit-llvm ../../lib/sylib.c -o " + id + ".sylib.ll")
                   .c_str()));
    ASSERT_FALSE(system(("llvm-link -S " + ll + " " + id + ".sylib.ll -o " +
                         id + ".final.ll")
                            .c_str()));
    int code;
    std::string io;
    if (fs::exists(in)) {
        io += " < ";
        io += in;
    }
    io += " > ";
    io += id;
    io += ".out";

    if (interpret) {
        code = system(("lli " + id + ".final.ll" + io).c_str());
    } else {
        ASSERT_FALSE(system(("llc " + id + ".final.ll -o " + id + ".s").c_str()));
        ASSERT_FALSE(system(("gcc " + id + ".s -no-pie -o " + id).c_str()));
        code = system(("./" + id + io).c_str());
    }
    code = WEXITSTATUS(code);
    String actual = read(id + ".out");
    String expected = read(out);
    if (!actual.empty() && actual.back() != '\n') {
        actual += "\n";
    }
    actual += std::to_string(code);
    actual += "\n";

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

String get_id(String path) {
    for (char &ch : path) {
        if (ch == '/') {
            ch = '_';
        }
    }
    return path;
}

void run_sysy(String path) {

    // in ./build/test
    path = "../../" + path;
    ASSERT_FALSE(
        system(("../frontend/SysYFrontend " + path + ".sy").c_str()));

    auto id = get_id(path.substr(6));

    printf("running %s.sy\n", path.c_str());
    judge(id, path + ".sy.ll", path + ".in", path + ".out");
}
