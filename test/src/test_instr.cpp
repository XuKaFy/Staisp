#include "gtest/gtest.h"
#include "ir_instr.h"
#include "ir_call_instr.h"
#include "ir_cast_instr.h"
#include "ir_cmp_instr.h"
#include "ir_control_instr.h"
#include "ir_constant.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "ir_ptr_instr.h"

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
    pType ti1 = make_basic_type(IMM_I1, false);
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
    pInstr arg_i1 = make_const_arg(TypedSym("arg_i1", ti1));

    auto c1 = make_cast_instr(ti32, arg_i64); c1->line = 1;
    auto c2 = make_cast_instr(ti64, arg_i32); c2->line = 2;
    auto c3 = make_cast_instr(ti64, arg_ptr); c3->line = 3;
    auto c4 = make_cast_instr(tptr, arg_i64); c4->line = 4;
    auto c5 = make_cast_instr(ti64, arg_f32); c5->line = 5;
    auto c6 = make_cast_instr(tf32, arg_i64); c6->line = 6;
    auto c7 = make_cast_instr(tu64, arg_f32); c7->line = 7;
    auto c8 = make_cast_instr(tf32, arg_u32); c8->line = 8;
    auto c9 = make_cast_instr(tu64, arg_u32); c9->line = 9;
    auto c10 = make_cast_instr(tu64, arg_i1); c10->line = 10;
    auto c11 = make_cast_instr(ti64, arg_i1); c11->line = 11;

    EXPECT_STREQ("%1 = trunc i64 %arg_i64 to i32", c1->instr_print());
    EXPECT_STREQ("%2 = sext i32 %arg_i32 to i64", c2->instr_print());
    EXPECT_STREQ("%3 = ptrtoint double* %arg_ptr to i64", c3->instr_print());
    EXPECT_STREQ("%4 = inttoptr i64 %arg_i64 to double*", c4->instr_print());
    EXPECT_STREQ("%5 = fptosi float %arg_f32 to i64", c5->instr_print());
    EXPECT_STREQ("%6 = sitofp i64 %arg_i64 to float", c6->instr_print());
    EXPECT_STREQ("%7 = fptoui float %arg_f32 to i64", c7->instr_print());
    EXPECT_STREQ("%8 = uitofp i32 %arg_u32 to float", c8->instr_print());
    EXPECT_STREQ("%9 = zext i32 %arg_u32 to i64", c9->instr_print());
    EXPECT_STREQ("%10 = zext i1 %arg_i1 to i64", c10->instr_print());
    EXPECT_STREQ("%11 = zext i1 %arg_i1 to i64", c11->instr_print());

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

// ir_cmp_instr.h

TEST(test_instr, cmp_instr) {
    pType ti32 = make_basic_type(IMM_I32, false);
    pInstr arg_i32_1 = make_const_arg(TypedSym("arg_i32_1", ti32));
    pInstr arg_i32_2 = make_const_arg(TypedSym("arg_i32_2", ti32));

    pType tf32 = make_basic_type(IMM_F32, false);
    pInstr arg_f32_1 = make_const_arg(TypedSym("arg_f32_1", tf32));
    pInstr arg_f32_2 = make_const_arg(TypedSym("arg_f32_2", tf32));

    auto c1 = make_cmp_instr(CMP_EQ, ti32, arg_i32_1, arg_i32_2); c1->line = 1;
    auto c2 = make_cmp_instr(CMP_EQ, tf32, arg_f32_1, arg_f32_2); c2->line = 2;

    EXPECT_STREQ("%1 = icmp eq i32 %arg_i32_1, %arg_i32_2", c1->instr_print());
    EXPECT_STREQ("%2 = fcmp eq float %arg_f32_1, %arg_f32_2", c2->instr_print());
}

// ir_control_instr.h

TEST(test_instr, label_instr) {
    pInstr i = make_label_instr();
    i->line = 100;
    EXPECT_STREQ("100:", i->instr_print());
}

TEST(test_instr, br_instr) {
    pInstr i = make_label_instr();
    i->line = 100;
    pInstr br = make_br_instr(i);
    EXPECT_STREQ("br label %100", br->instr_print());
}

TEST(test_instr, br_cond_instr) {
    pInstr arg1 = make_const_arg(TypedSym("cond", make_basic_type(IMM_I32, false)));
    pInstr i = make_label_instr();
    i->line = 100;
    pInstr j = make_label_instr();
    j->line = 200;
    pInstr br = make_br_cond_instr(arg1, i, j);
    EXPECT_STREQ("br i32 %cond, label %100, label %200", br->instr_print());
}

// ir_mem_instr.h

TEST(test_instr, alloc_instr) {
    pType elem_type = make_basic_type(IMM_F64, false);
    pInstr i = make_alloc_instr(elem_type);
    i->line = 200;
    EXPECT_TRUE(is_same_type(to_elem_type(i->tr), elem_type));
    EXPECT_STREQ("%200 = alloca double", i->instr_print());
}

TEST(test_instr, load_instr) {
    pType elem_type = make_basic_type(IMM_F64, false);
    pInstr alloc = make_alloc_instr(elem_type);
    alloc->line = 200;
    pInstr i = make_load_instr(alloc);
    i->line = 300;
    EXPECT_TRUE(is_same_type(i->tr, elem_type));
    EXPECT_STREQ("%300 = load double, double* %200", i->instr_print());
}

TEST(test_instr, store_instr) {
    pType elem_type = make_basic_type(IMM_F64, false);
    pInstr alloc = make_alloc_instr(elem_type);
    alloc->line = 200;
    pInstr arg1 = make_const_arg(TypedSym("arg1", make_basic_type(IMM_F64, false)));
    pInstr i = make_store_instr(alloc, arg1);
    EXPECT_STREQ("store double %arg1, double* %200", i->instr_print());
}

// ir_opr_instr.h

TEST(test_instr, binary_opr_instr) {
    pType i32 = make_basic_type(IMM_I32, false);
    pInstr arg1 = make_const_arg(TypedSym("arg1", i32));
    pInstr arg2 = make_const_arg(TypedSym("arg2", i32));
    pInstr i = make_binary_instr(INSTR_DIV, i32, arg1, arg2);
    i->line = 999;
    EXPECT_STREQ("%999 = sdiv i32 %arg1, %arg2", i->instr_print());

    pType f32 = make_basic_type(IMM_F32, false);
    pInstr arg3 = make_const_arg(TypedSym("arg3", f32));
    pInstr arg4 = make_const_arg(TypedSym("arg4", f32));
    pInstr j = make_binary_instr(INSTR_DIV, f32, arg3, arg4);
    j->line = 998;
    EXPECT_STREQ("%998 = fdiv float %arg3, %arg4", j->instr_print());

    pType u32 = make_basic_type(IMM_U32, false);
    pInstr arg5 = make_const_arg(TypedSym("arg5", u32));
    pInstr arg6 = make_const_arg(TypedSym("arg6", u32));
    pInstr k = make_binary_instr(INSTR_DIV, u32, arg5, arg6);
    k->line = 997;
    EXPECT_STREQ("%997 = udiv i32 %arg5, %arg6", k->instr_print());
}

TEST(test_instr, unary_opr_instr) {
    pType i32 = make_basic_type(IMM_I32, false);
    pInstr arg1 = make_const_arg(TypedSym("arg1", i32));
    pInstr i = make_unary_instr(INSTR_NOT, i32, arg1);
    i->line = 999;
    EXPECT_STREQ("%999 = not i32 %arg1", i->instr_print());
}

// ir_ptr_instr.h

TEST(test_instr, item_instr) {
    pType i32 = make_basic_type(IMM_I32, false);
    pType arr = make_array_type(make_array_type(i32, 10), 10);
    pInstr arg = make_const_arg(TypedSym("arg", make_pointer_type(arr, false)));
    EXPECT_STREQ(arg->tr->type_name(), "[10 x [10 x i32]]*");

    pInstr a1 = make_binary_instr(INSTR_ADD, i32, 
        make_constant(ImmValue(2ll, IMM_I32)),
        make_constant(ImmValue(3ll, IMM_I32))
    );
    a1->line = 2;
    EXPECT_STREQ(a1->tr->type_name(), "i32");

    pInstr i = make_item_instr(arg, { a1, a1 });
    i->line = 500;
    EXPECT_TRUE(is_same_type(i->tr, make_pointer_type(i32, false)));
    EXPECT_STREQ("%500 = getelementptr [10 x [10 x i32]], [10 x [10 x i32]]* %arg, i32 0, i32 %2, i32 %2", i->instr_print());
}
