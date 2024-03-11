#include "gtest/gtest.h"
#include "ir_block.h"
#include "ir_func_defined.h"
#include "ir_module.h"

using namespace Ir;

TEST(test_block, basic_block) {
    pBlock b = make_block();
    size_t line = 10;
    b->generate_line(line);
    EXPECT_STREQ(b->print_block(), "10:\n");

    b = make_block(); // 如果还用原来的块，那么 label 始终是 10 不会变，不知道为什么
    // 说明块不应该多次调用 generate line
    line = 1;

    pBlock c = make_block();
    b->finish_block_with_jump(c);
    b->generate_line(line);
    c->generate_line(line);
    EXPECT_STREQ(b->print_block(), "1:\n  br label %2\n");    
}

TEST(test_func_defined, basic_func_defined) {
    pType i32 = make_basic_type(IMM_I32, false);
    pType i32c = make_basic_type(IMM_I32, true);
    TypedSym var = TypedSym("fun", i32);

    pFuncDefined func = make_func_defined(var, { TypedSym("arg1", i32), TypedSym("arg2", i32c) });
    EXPECT_EQ(func->body.size(), 1);
    // 其他的很难测试，先不测试
}

TEST(test_module, basic_module) {
    // 先不测试
}
