#include "read_file.h"

#include "gtest/gtest.h"
#include <fstream>

pCode read_test_file(const String &file_suffix) {
    String file = String("../../code/") + file_suffix;
    std::ifstream in;
    in.open(file, std::fstream::in);
    EXPECT_TRUE(in.is_open());

    String code((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    pCode p_code = make_code(code, file);
    return p_code;
}
