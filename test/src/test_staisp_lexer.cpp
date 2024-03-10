#include "gtest/gtest.h"
#include "staisp_lexer.h"

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
