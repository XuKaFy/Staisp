#pragma once

#include "def.h"

#include "gtest/gtest.h"
#include <fstream>

#include <cstdlib>
#include <string>

String read(String file)
{
    std::ifstream in;
    in.open(file, std::fstream::in);
    EXPECT_TRUE(in.is_open());

    String code((std::istreambuf_iterator<char>(in)),  
                 std::istreambuf_iterator<char>());
    return code;
}

void prepare()
{
    static bool flag = false;
    if(!flag) {
        EXPECT_FALSE(system("clang -S -emit-llvm ../../lib/sylib.c"));
        EXPECT_FALSE(system("cp ../../lib/sylib.ll sylib.ll"));
        EXPECT_FALSE(system("llvm-as sylib.ll -o sylib.bc"));
        flag = true;
    }
}

std::string normalizeLineEndings(const std::string& str) {
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

void cmp(String ll)
{
    prepare();
    system((String("llvm-as ") + ll + " -o out.bc").c_str());
    system("llvm-link out.bc sylib.bc -o final.bc");
    system("lli final.bc < 1.std.in > 1.out ; echo $? > 1.out.ret");
    String actual = read("1.out");
    String tret = read("1.out.ret");
    String expected = read("1.std.out");
    if(!actual.empty() && actual.back() != '\n') actual += "\n";
    actual += tret;
    EXPECT_EQ(normalizeLineEndings(actual), normalizeLineEndings(expected));
}

void run_sysy(String file)
{
    // in ./build/test
    int res = system((String("../frontend/SysYFrontend ../../") + file + ".sy").c_str());
    if(res) {
        EXPECT_EQ(0, 1);
        return ;
    }
    system((String("cp ../../") + file + ".out 1.std.out").c_str());
    system((String("cp ../../") + file + ".in 1.std.in").c_str());
    system((String("cp ../../") + file + ".sy.opt.ll 1.sy.opt.ll").c_str());
    system((String("cp ../../") + file + ".sy.ll 1.sy.ll").c_str());
    printf("running %s.sy\n", file.c_str());
    cmp("1.sy.ll");
    // cmp("1.sy.opt.ll");
}

void run_staisp(String file)
{
    int res = system((String("../staisp_frontend ../..") + file + ".sta").c_str());
    if(res) {
        EXPECT_EQ(0, 1);
        return ;
    }
    system((String("cp ../../") + file + ".out 1.std.out").c_str());
    system((String("cp ../../") + file + ".in 1.std.in").c_str());
    system((String("cp ../../") + file + ".sta.opt.ll 1.sta.opt.ll").c_str());
    system((String("cp ../../") + file + ".sta.ll 1.sta.ll").c_str());
    printf("running %s.sta\n", file.c_str());
    cmp("1.sta.ll");
    cmp("1.sta.opt.ll");
}
