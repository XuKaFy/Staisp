#include "read_file.h"

#include "gtest/gtest.h"
#include <fstream>

/* TODO
    写好看一点
    加上库之后就写成对比 in、out 就行了。
*/

pCode read_test_file(Symbol file_suffix) {
    String file = String("../../code/") + file_suffix;
    std::ifstream in;
    in.open(file, std::fstream::in);
    EXPECT_TRUE(in.is_open());

    String code((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    pCode p_code = make_code(code, file);
    return p_code;
}
