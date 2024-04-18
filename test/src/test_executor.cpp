#include "gtest/gtest.h"
#include "staisp_parser.h"
#include "convert_ast_to_ir.h"
#include "read_file.h"

#define TEST_A_FILE(x) \
TEST(test_executor, executor_error_##x) { \
    pCode p_code = read_test_file("error_executor/" #x ".sta"); \
    try { \
        AstProg root = Staisp::Parser().parse(p_code); \
        Ir::pModule mod = AstToIr::Convertor().generate(root); \
        EXPECT_TRUE(false); \
    } catch (Exception e) { \
        EXPECT_EQ(e.id, x); \
        EXPECT_STREQ(e.object, "Executor"); \
    } \
}

// TEST_A_FILE(8)
