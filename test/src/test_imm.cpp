#include "imm.h"
#include "gtest/gtest.h"

TEST(test_imm, imm_map) {
    EXPECT_EQ(gSymToImmType.find("f64")->second, IMM_F64);
    EXPECT_EQ(gSymToImmType.find("f32")->second, IMM_F32);
    EXPECT_EQ(gSymToImmType.find("i64")->second, IMM_I64);
    EXPECT_EQ(gSymToImmType.find("u64")->second, IMM_U64);
}

TEST(test_imm, imm_utility) {
    // float
    EXPECT_TRUE(is_imm_float(IMM_F64));
    EXPECT_TRUE(is_imm_float(IMM_F32));
    EXPECT_FALSE(is_imm_float(IMM_I64));
    EXPECT_FALSE(is_imm_float(IMM_U64));
    // integer
    EXPECT_TRUE(is_imm_integer(IMM_I64));
    EXPECT_TRUE(is_imm_integer(IMM_U64));
    EXPECT_FALSE(is_imm_integer(IMM_F64));
    EXPECT_FALSE(is_imm_integer(IMM_F32));
    // signed type
    EXPECT_TRUE(is_imm_signed(IMM_F64));
    EXPECT_TRUE(is_imm_signed(IMM_F32));
    EXPECT_TRUE(is_imm_signed(IMM_I64));
    EXPECT_FALSE(is_imm_signed(IMM_U64));
    // bytes of type
    EXPECT_EQ(bytes_of_imm_type(IMM_I64), 8);
    EXPECT_EQ(bytes_of_imm_type(IMM_U64), 8);
    EXPECT_EQ(bytes_of_imm_type(IMM_F64), 8);
    EXPECT_EQ(bytes_of_imm_type(IMM_F32), 4);
    EXPECT_EQ(bytes_of_imm_type(IMM_I32), 4);
    EXPECT_EQ(bytes_of_imm_type(IMM_U32), 4);
    EXPECT_EQ(bytes_of_imm_type(IMM_I8), 1);
    EXPECT_EQ(bytes_of_imm_type(IMM_I1), 1);
    // join type
    EXPECT_EQ(join_imm_type(IMM_I32, IMM_I64), IMM_I64);
    EXPECT_EQ(join_imm_type(IMM_F32, IMM_F32), IMM_F32);
    EXPECT_EQ(join_imm_type(IMM_I32, IMM_I32), IMM_I32);
    EXPECT_EQ(join_imm_type(IMM_I64, IMM_I64), IMM_I64);
    EXPECT_EQ(join_imm_type(IMM_F64, IMM_F64), IMM_F64);
    EXPECT_EQ(join_imm_type(IMM_I32, IMM_U32), IMM_U32);
    EXPECT_EQ(join_imm_type(IMM_I32, IMM_F32), IMM_F32); // ?
    EXPECT_EQ(join_imm_type(IMM_F32, IMM_F64), IMM_F64); // ?
}

TEST(test_imm, imm_opr_int) {
    ImmValue a(13);
    ImmValue b(4);
    EXPECT_EQ((a + b).val.ival, 17);
    EXPECT_EQ((a - b).val.ival, 9);
    EXPECT_EQ((a * b).val.ival, 52);
    EXPECT_EQ((a / b).val.ival, 3);
    EXPECT_EQ((a % b).val.ival, 1);
    EXPECT_EQ((a && b).val.ival, 1);
    EXPECT_EQ((a || b).val.ival, 1);
    EXPECT_EQ((a & b).val.ival, 4);
    EXPECT_EQ((a | b).val.ival, 13);
    EXPECT_EQ((a < b).val.ival, 0);
    EXPECT_EQ((a > b).val.ival, 1);
    EXPECT_EQ((a <= b).val.ival, 0);
    EXPECT_EQ((a >= b).val.ival, 1);
    EXPECT_EQ((a == b).val.ival, 0);
    EXPECT_EQ((a != b).val.ival, 1);
}

TEST(test_imm, imm_opr_float) {
    ImmValue a(13.0);
    ImmValue b(4.0);
    EXPECT_EQ((a + b).val.f64val, 17);
    EXPECT_EQ((a - b).val.f64val, 9);
    EXPECT_EQ((a * b).val.f64val, 52);
    EXPECT_EQ((a / b).val.f64val, 3.25);
    try {
        (a % b).val.f64val;
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 2);
        EXPECT_STREQ(e.object.c_str(), "ImmValue");
    }
    EXPECT_EQ((a && b).val.ival, 1);
    EXPECT_EQ((a || b).val.ival, 1);
    try {
        (a & b).val.f64val;
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 2);
        EXPECT_STREQ(e.object.c_str(), "ImmValue");
    }
    try {
        (a | b).val.f64val;
        EXPECT_TRUE(false);
    } catch (Exception e) {
        EXPECT_EQ(e.id, 2);
        EXPECT_STREQ(e.object.c_str(), "ImmValue");
    }
    EXPECT_EQ((a < b).val.ival, 0);
    EXPECT_EQ((a > b).val.ival, 1);
    EXPECT_EQ((a <= b).val.ival, 0);
    EXPECT_EQ((a >= b).val.ival, 1);
    EXPECT_EQ((a == b).val.ival, 0);
    EXPECT_EQ((a != b).val.ival, 1);
}
