#include "gtest/gtest.h"
#include "staisp_lexer.h"
#include "read_file.h"

#include <fstream>

TEST(test_staisp_lexer, basic_lexer) {
    Staisp::Lexer lexer;
    TokenList ans;
    pCode code;

    code = make_code("TOKEN1 TOKEN2 TOKEN3 1234567", "test1");
    EXPECT_NO_THROW(ans = lexer.lexer(code));
    EXPECT_EQ(ans.size(), 4);

    code = make_code("TOKEN1 TOKEN2 TOKEN3 1234567abcd", "test1");
    try {
        ans = lexer.lexer(code);
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_TRUE(strcmp(e.object, "Lexer") == 0);
    }
}

TEST(test_staisp_lexer, lexer_error_1) {
    Staisp::Lexer lexer;

    pCode p_code = read_file("error_lexer/1.sta");

    try {
        lexer.lexer(p_code);
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_TRUE(strcmp(e.object, "Lexer") == 0);
    }
}
