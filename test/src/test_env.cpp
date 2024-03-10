#include "gtest/gtest.h"
#include "env.h"

TEST(test_env, basic_env) {
    Env<int> a;
    try {
        a.find("hello");
        EXPECT_TRUE(false);
    } catch(Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_TRUE(strcmp(e.object, "Env") == 0);
    }
    a.set("hello", 10);
    try {
        EXPECT_EQ(a.find("hello"), 10);
    } catch(Exception e) {
        EXPECT_TRUE(false);
    }
    try {
        a.find("hello_world");
        EXPECT_TRUE(false);
    } catch(Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_TRUE(strcmp(e.object, "Env") == 0);
    }
}

TEST(test_env_wrapper, basic_env_wrapper) {
    EnvWrapper<int> ew;
    EXPECT_EQ(ew.env_count(), 0);
    try {
        ew.env();
    } catch (Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_TRUE(strcmp(e.object, "EnvWrapper") == 0);
    }
    ew.push_env();
    EXPECT_EQ(ew.env_count(), 1);
    ew.env()->set("hello", 10);
    try {
        EXPECT_EQ(ew.env()->find("hello"), 10);
    } catch (Exception e) {
        EXPECT_TRUE(false);
    }
    ew.push_env();
    EXPECT_EQ(ew.env_count(), 2);
    try {
        EXPECT_EQ(ew.env()->find("hello"), 10);
    } catch (Exception e) {
        EXPECT_TRUE(false);
    }
    ew.end_env();
    EXPECT_EQ(ew.env_count(), 1);
    ew.clear_env();
    EXPECT_EQ(ew.env_count(), 0);
    try {
        ew.end_env();
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 2);
        EXPECT_TRUE(strcmp(e.object, "EnvWrapper") == 0);
    }
}
