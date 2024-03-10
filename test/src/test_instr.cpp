#include "gtest/gtest.h"
#include "ir_instr.h"
#include "ir_call_instr.h"
#include "ir_cast_instr.h"

using namespace Ir;

// ir_instr.h

TEST(test_instr, empty_instr) {
    pInstr i = make_empty_instr();
    EXPECT_FALSE(i);
}

TEST(test_instr, const_arg) {
    pInstr i = make_const_arg(TypedSym("var", make_basic_type(IMM_I32, false)));
    i->line = 10;
    EXPECT_STREQ("%var", i->print());
    EXPECT_TRUE(is_same_type(i->tr, make_basic_type(IMM_I32, false)));
}

// ir_call_instr.h

TEST(test_instr, call_instr) {
    pFunc func = make_func(TypedSym("func", make_basic_type(IMM_I32, false)), { 
        TypedSym("varg1", make_basic_type(IMM_I32, false)),
        TypedSym("varg2", make_basic_type(IMM_I32, true)),
    });
    pInstr arg1 = make_const_arg(TypedSym("arg1", make_basic_type(IMM_I32, false)));
    pInstr arg2 = make_const_arg(TypedSym("arg2", make_basic_type(IMM_I32, true)));
    pInstr i = make_call_instr(func, { arg1, arg2 });
    i->line = 10;
    printf("instr: %s\n", i->instr_print());
    // %10 = call i32 @@func(i32 %arg1, i32 %arg2)
    EXPECT_STREQ("%10 = call i32 @func(i32 %arg1, i32 %arg2)", i->instr_print());
    EXPECT_TRUE(is_same_type(i->tr, make_basic_type(IMM_I32, false)));    
}

TEST(test_instr, ret_instr) {
    pInstr arg = make_const_arg(TypedSym("arg", make_basic_type(IMM_I32, false)));
    pInstr i = make_ret_instr(make_basic_type(IMM_I32, false), arg);
    EXPECT_STREQ("ret i32 %arg", i->instr_print());
    EXPECT_TRUE(is_same_type(i->tr, make_basic_type(IMM_I32, false)));  
}

// ir_cast_instr.h

TEST(test_instr, cast_instr) {
    pType ti32 = make_basic_type(IMM_I32, false);
    pType ti64 = make_basic_type(IMM_I64, false);
    pType tu32 = make_basic_type(IMM_U32, false);
    pType tu64 = make_basic_type(IMM_U64, false);
    pType tf32 = make_basic_type(IMM_F32, false);
    pType tptr = make_pointer_type(make_basic_type(IMM_F64, false), false);
    pType tarr = make_array_type(make_basic_type(IMM_F64, false), 10);

    pInstr arg_i32 = make_const_arg(TypedSym("arg_i32", ti32));
    pInstr arg_i64 = make_const_arg(TypedSym("arg_i64", ti64));
    pInstr arg_u32 = make_const_arg(TypedSym("arg_u32", tu32));
    pInstr arg_f32 = make_const_arg(TypedSym("arg_f32", tf32));
    pInstr arg_ptr = make_const_arg(TypedSym("arg_ptr", tptr));

    auto c1 = make_cast_instr(ti32, arg_i64); c1->line = 1;
    auto c2 = make_cast_instr(ti64, arg_i32); c2->line = 2;
    auto c3 = make_cast_instr(ti64, arg_ptr); c3->line = 3;
    auto c4 = make_cast_instr(tptr, arg_i64); c4->line = 4;
    auto c5 = make_cast_instr(ti64, arg_f32); c5->line = 5;
    auto c6 = make_cast_instr(tf32, arg_i64); c6->line = 6;
    auto c7 = make_cast_instr(tu64, arg_f32); c7->line = 7;
    auto c8 = make_cast_instr(tf32, arg_u32); c8->line = 8;
    auto c9 = make_cast_instr(tu64, arg_u32); c9->line = 9;

    EXPECT_STREQ("%1 = trunc i64 %arg_i64 to i32", c1->instr_print());
    EXPECT_STREQ("%2 = sext i32 %arg_i32 to i64", c2->instr_print());
    EXPECT_STREQ("%3 = ptrtoint double* %arg_ptr to i64", c3->instr_print());
    EXPECT_STREQ("%4 = inttoptr i64 %arg_i64 to double*", c4->instr_print());
    EXPECT_STREQ("%5 = fptosi float %arg_f32 to i64", c5->instr_print());
    EXPECT_STREQ("%6 = sitofp i64 %arg_i64 to float", c6->instr_print());
    EXPECT_STREQ("%7 = fptoui float %arg_f32 to i64", c7->instr_print());
    EXPECT_STREQ("%8 = uitofp i32 %arg_u32 to float", c8->instr_print());
    EXPECT_STREQ("%9 = zext i32 %arg_u32 to i64", c9->instr_print());

    try {
        make_cast_instr(ti32, arg_i64);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 1);
        EXPECT_STREQ(e.object, "CastInstr");
    }
    try {
        make_cast_instr(tarr, arg_i64);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 2);
        EXPECT_STREQ(e.object, "CastInstr");
    }
}
