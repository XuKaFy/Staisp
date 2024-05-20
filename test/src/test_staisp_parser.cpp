#include "read_file.h"
#include "staisp_parser.h"
#include "gtest/gtest.h"

#define TEST_A_FILE(x)                                                         \
    TEST(test_staisp_parser, parser_error_##x) {                               \
        Staisp::Parser parser;                                                 \
        pCode p_code = read_test_file("error_parser/" #x ".sta");              \
        try {                                                                  \
            parser.parse(p_code);                                              \
            EXPECT_TRUE(false);                                                \
        } catch (Exception e) {                                                \
            EXPECT_EQ(e.id, x);                                                \
            EXPECT_STREQ(e.object.c_str(), "Parser");                          \
        }                                                                      \
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
TEST_A_FILE(12)
// TEST_A_FILE(13)
