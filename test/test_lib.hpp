#pragma once

#include <chrono>

class Stopwatch {
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

    template<typename F>
    double timeit(F&& f) {
        reset();
        std::invoke(std::forward<F>(f));
        return stop();
    }
};

inline std::string read(const std::string &file) {
    FILE* input_file = fopen(file.c_str(), "rb");
    fseek(input_file, 0, SEEK_END);
    size_t size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    std::string fileBuffer(size, '\0');
    fread(fileBuffer.data(), 1, size, input_file);
    fclose(input_file);
    return fileBuffer;
}

inline std::string normalizeLineEndings(const std::string &str) {
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


inline std::string get_id(std::string path) {
    for (char &ch : path) {
        if (ch == '/') {
            ch = '_';
        }
    }
    return path;
}

inline std::string get_path(const std::string &filename) {
    // pwd: ./build/test
    return "../../" + filename;
}