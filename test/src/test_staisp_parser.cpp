#include "gtest/gtest.h"
#include "staisp_parser.h"
#include "read_file.h"

#define TEST_A_FILE(x) \
TEST(test_staisp_parser, parser_error_##x) { \
    Staisp::Parser parser; \
    pCode p_code = read_test_file("error_parser/" #x ".sta"); \
    try { \
        parser.parse(p_code); \
        EXPECT_TRUE(false); \
    } catch (Exception e) { \
        EXPECT_EQ(e.id, x); \
        EXPECT_TRUE(strcmp(e.object, "Parser") == 0); \
    } \
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
