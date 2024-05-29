#include "read_file.h"

#include "gtest/gtest.h"

#include "convert_ast_to_ir.h"
#include "staisp_parser.h"

#include "opt.h"
#include <fstream>

#if 0

#define TEST_A_FILE(x)                                                         \
    TEST(test_minimal_code, code_##x) {                                        \
        pCode p_code = read_test_file("./minimal_code/" #x ".sta");            \
        AstProg root = Staisp::Parser().parse(p_code);                         \
        Ir::pModule mod = AstToIr::Convertor().generate(root);                 \
        std::ofstream out;                                                     \
        out.open("../../code/minimal_code/" #x ".sta.ll", std::fstream::out);  \
        out << mod->print_module();                                            \
        out.close();                                                           \
        EXPECT_EQ(system("lli ../../code/minimal_code/" #x ".sta.ll"), 0);     \
        Optimize::optimize(mod);                                               \
        out.open("../../code/minimal_code/" #x ".sta.opt.ll",                  \
                 std::fstream::out);                                           \
        out << mod->print_module();                                            \
        out.close();                                                           \
        EXPECT_EQ(system("lli ../../code/minimal_code/" #x ".sta.opt.ll"), 0); \
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
TEST_A_FILE(14)
TEST_A_FILE(15)
TEST_A_FILE(16)
TEST_A_FILE(17)
TEST_A_FILE(18)

#endif