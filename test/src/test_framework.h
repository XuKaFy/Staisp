#pragma once

#include "def.h"

#include "gtest/gtest.h"
#include <fstream>

#include <cstdlib>
#include <string>

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

void judge(const String &id) {
    ASSERT_FALSE(
        system(("clang -S -emit-llvm ../../lib/sylib.c -o " + id + ".sylib.ll")
                   .c_str()));
    ASSERT_FALSE(
        system(("llvm-as " + id + ".sylib.ll -o " + id + ".sylib.bc").c_str()));
    ASSERT_FALSE(
        system(("llvm-as " + id + ".sy.ll -o " + id + ".sy.bc").c_str()));
    ASSERT_FALSE(system(("llvm-link " + id + ".sy.bc " + id + ".sylib.bc -o " +
                         id + "_final.bc")
                            .c_str()));
    int code;
    if (interpret) {
        code = system(
            ("lli " + id + "_final.bc < " + id + ".std.in > " + id + ".out")
                .c_str());
    } else {
        ASSERT_FALSE(
            system(("llc " + id + "_final.bc -o " + id + ".s").c_str()));
        ASSERT_FALSE(system(("gcc " + id + ".s -no-pie -o " + id).c_str()));
        code = system(
            ("./" + id + " < " + id + ".std.in > " + id + ".out").c_str());
    }
    code = WEXITSTATUS(code);
    String actual = read(id + ".out");
    String expected = read(id + ".std.out");
    if (!actual.empty() && actual.back() != '\n') {
        actual += "\n";
    }
    actual += std::to_string(code);
    actual += "\n";

    // always remove
    remove((id + ".sylib.ll").c_str());
    remove((id + ".sylib.bc").c_str());
    remove((id + ".sy.bc").c_str());
    remove((id + ".std.in").c_str());
    remove((id + ".std.out").c_str());

    ASSERT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));

    // remove if success
    remove((id + ".out").c_str());
    remove((id + ".sy.ll").c_str());
    remove((id + ".s").c_str());
    remove((id + "_final.bc").c_str());
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

void mov(const String &path, const String &id, const String &suffix1,
         const String &suffix2) {
    std::string command;
    command += "cp ../../";
    command += path;
    command += suffix1;
    command += " ";
    command += id;
    command += suffix2;
    if (system(command.c_str()) != 0) {
        system(("touch " + id + suffix2).c_str());
    }
}

void run_sysy(const String &path) {
    // in ./build/test
    ASSERT_FALSE(
        system(("../frontend/SysYFrontend ../../" + path + ".sy").c_str()));

    auto id = get_id(path);

    mov(path, id, ".out", ".std.out");
    mov(path, id, ".in", ".std.in");
    mov(path, id, ".sy.ll", ".sy.ll");
    printf("running %s.sy\n", path.c_str());
    judge(id);
}
