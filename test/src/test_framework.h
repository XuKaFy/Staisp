#pragma once

#include "def.h"

#include "gtest/gtest.h"

#include <chrono>
#include <cstdlib>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

String read(const String &file) {
    FILE* input_file = fopen(file.c_str(), "rb");
    fseek(input_file, 0, SEEK_END);
    size_t size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    std::string fileBuffer(size, '\0');
    fread(fileBuffer.data(), 1, size, input_file);
    fclose(input_file);
    return fileBuffer;
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

class Stopwatch {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;

public:

    void reset() {
        start_time = std::chrono::steady_clock::now();
    }

    double stop() {
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = end_time - start_time;
        return elapsed_time.count();
    }
};


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

    // pwd: ./build/test
    path = "../../" + path;
    auto id = get_id(path.substr(6));

    Stopwatch stopwatch;

    stopwatch.reset();

    ASSERT_FALSE(
        system(("../frontend/SysYFrontend " + path + ".sy").c_str()));

    double t1 = stopwatch.stop();

    stopwatch.reset();
    judge(id, path + ".sy.ll", path + ".in", path + ".out");
    double t2 = stopwatch.stop();

    // if you hope to watch detailed time info, use following command:
    // ctest -V | grep sysy_tests
    printf("\t| %-55s | %5.2fs | %5.2fs | \n", id.c_str(), t1, t2);
}
