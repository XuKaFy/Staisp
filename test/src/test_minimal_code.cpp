#include "read_file.h"

#include "staisp_parser.h"
#include "convert_ast_to_ir.h"

#define TEST_A_FILE(x) \
TEST(test_minimal_code, code_##x) { \
    pCode p_code = read_test_file("minimal_code/" #x ".sta"); \
    AstProg root = Staisp::Parser().parse(p_code); \
    Ir::pModule mod = AstToIr::Convertor().generate(root); \
    std::ofstream out; \
    out.open("test.ll", std::fstream::out); \
    out << mod->print_module(); \
    out.close(); \
    EXPECT_EQ(system("lli test.ll"), 0); \
}

TEST_A_FILE(1)
TEST_A_FILE(2)
TEST_A_FILE(3)
TEST_A_FILE(4)
TEST_A_FILE(5)
TEST_A_FILE(6)
TEST_A_FILE(7)
TEST_A_FILE(8)
TEST_A_FILE(9)
TEST_A_FILE(10)
TEST_A_FILE(11)
TEST_A_FILE(12)
TEST_A_FILE(13)
