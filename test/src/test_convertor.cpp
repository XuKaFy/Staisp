#include "gtest/gtest.h"
#include "staisp_parser.h"
#include "convert_ast_to_ir.h"
#include "read_file.h"

#define TEST_A_FILE(x) \
TEST(test_convertor, convertor_error_##x) { \
    pCode p_code = read_test_file("error_convertor/" #x ".sta"); \
    try { \
        AstProg root = Staisp::Parser().parse(p_code); \
        Ir::pModule mod = AstToIr::Convertor().generate(root); \
        EXPECT_TRUE(false); \
    } catch (Exception e) { \
        EXPECT_EQ(e.id, x); \
        EXPECT_STREQ(e.object, "Convertor"); \
    } \
}

#define TEST_A_FILE_2(x, y) \
TEST(test_convertor, convertor_error__##x##y) { \
    pCode p_code = read_test_file("error_convertor/" #x "_" #y ".sta"); \
    try { \
        AstProg root = Staisp::Parser().parse(p_code); \
        Ir::pModule mod = AstToIr::Convertor().generate(root); \
        EXPECT_TRUE(false); \
    } catch (Exception e) { \
        EXPECT_EQ(e.id, x); \
        EXPECT_STREQ(e.object, "Convertor"); \
    } \
}

TEST_A_FILE(1)
TEST_A_FILE(3)
TEST_A_FILE(5)
TEST_A_FILE(8)
TEST_A_FILE(9)
TEST_A_FILE(10)
TEST_A_FILE(11)
TEST_A_FILE_2(10, 2)
TEST_A_FILE_2(11, 2)
TEST_A_FILE_2(11, 3)
// TEST_A_FILE(19)
