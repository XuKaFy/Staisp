#include <memory>

#include "common_node.h"
#include "common_token.h"
#include "gtest/gtest.h"

#if 0

TEST(test_common_token, basic_token) {
    String s = "hello world";
    pCode pcode = make_code("hello world", "file1");
    Token t = Token(pcode, pcode->p_code->begin(), pcode->p_code->end(), 1);
    try {
        t.throw_error(10, "test", "null");
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_STREQ(e.object.c_str(), "test");
        EXPECT_STREQ(e.message.c_str(), "null");
        EXPECT_EQ(e.id, 10);
    }
}

TEST(test_common_node, basic_node) {
    String s = "hello world";
    pCode pcode = make_code("hello world", "file1");
    Node n = Node(std::make_shared<Token>(pcode, pcode->p_code->begin(),
                                          pcode->p_code->end(), 1),
                  NODE_ASSIGN);
    EXPECT_EQ(n.type, NODE_ASSIGN);
    try {
        n.throw_error(10, "test", "null");
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_STREQ(e.object.c_str(), "test");
        EXPECT_STREQ(e.message.c_str(), "null");
        EXPECT_EQ(e.id, 10);
    }
}

#endif