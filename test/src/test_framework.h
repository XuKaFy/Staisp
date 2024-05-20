#pragma once

#include "def.h"

#include "gtest/gtest.h"
#include <fstream>

#include <cstdlib>
#include <string>

String read(String file) {
    std::ifstream in;
    in.open(file, std::fstream::in);
    EXPECT_TRUE(in.is_open()) << file;

    String code((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    return code;
}

void prepare() {
    static bool flag = false;
    if (!flag) {
        EXPECT_FALSE(system("clang -S -emit-llvm ../../lib/sylib.c"));
        EXPECT_FALSE(system("llvm-as sylib.ll -o sylib.bc"));
        flag = true;
    }
}

std::string normalizeLineEndings(const std::string &str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\r') {
            if (i + 1 < str.size() && str[i + 1] == '\n') {
                result += '\n';
                ++i; // Skip '\n'
            } else {
                result += '\n';
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

void cmp(String id) {
    prepare();
    system(("llvm-as " + id + ".sy.ll -o " + id + "_out.bc").c_str());
    system(("llvm-link " + id + "_out.bc sylib.bc -o " + id + "_final.bc").c_str());
    system(("lli " + id + "_final.bc < " + id + ".std.in > " + id + ".out ; echo $? > " + id + ".out.ret").c_str());
    String actual = read(id + ".out");
    String tret = read(id + ".out.ret");
    String expected = read(id + ".std.out");
    if (!actual.empty() && actual.back() != '\n')
        actual += "\n";
    actual += tret;
    EXPECT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));
}

String get_id(String path) {
    for (char& ch : path) {
        if (ch == '/') ch = '_';
    }
    return path;
}

void mov(String path, String id, String suffix1, String suffix2) {
    std::string command;
    command += "cp ../../";
    command += path;
    command += suffix1;
    command += " ";
    command += id;
    command += suffix2;
    if (system(command.c_str())) {
        system(("touch " + id + suffix2).c_str());
    }
}

void run_sysy(String path) {
    // in ./build/test
    int res = system(("../frontend/SysYFrontend ../../"+ path + ".sy").c_str());
    ASSERT_EQ(res, 0);

    auto id = get_id(path);

    mov(path, id, ".out", ".std.out");
    mov(path, id, ".in", ".std.in");
    mov(path, id, ".sy.ll", ".sy.ll");
    printf("running %s.sy\n", path.c_str());
    cmp(id);
}

//void run_staisp(String file) {
//    int res =
//        system((String("../staisp_frontend ../..") + file + ".sta").c_str());
//    ASSERT_EQ(res, 0);
//    system((String("cp ../../") + file + ".out 1.std.out").c_str());
//    system((String("cp ../../") + file + ".in 1.std.in").c_str());
//    system((String("cp ../../") + file + ".sta.opt.ll 1.sta.opt.ll").c_str());
//    system((String("cp ../../") + file + ".sta.ll 1.sta.ll").c_str());
//    printf("running %s.sta\n", file.c_str());
//    cmp("1.sta.ll");
//    cmp("1.sta.opt.ll");
//}
