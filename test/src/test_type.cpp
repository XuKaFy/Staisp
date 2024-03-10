#include "gtest/gtest.h"
#include "type.h"

TEST(test_type, type_helper) {
    {
        pType t = make_pointer_type(make_basic_type(IMM_I32, false), true);
        EXPECT_FALSE(is_integer(t));
        EXPECT_FALSE(is_float(t));
        EXPECT_FALSE(is_signed_type(t));
        EXPECT_FALSE(is_array(t));
        EXPECT_FALSE(is_struct(t));
        EXPECT_TRUE(is_pointer(t));
        EXPECT_FALSE(is_basic_type(t));
        EXPECT_TRUE(is_const_type(t));
        EXPECT_EQ(bytes_of_type(t), ARCH_BYTES);
    }
    {
        pType t = make_basic_type(IMM_U32, false);
        EXPECT_TRUE(is_integer(t));
        EXPECT_FALSE(is_float(t));
        EXPECT_FALSE(is_signed_type(t));
        EXPECT_FALSE(is_array(t));
        EXPECT_FALSE(is_struct(t));
        EXPECT_FALSE(is_pointer(t));
        EXPECT_TRUE(is_basic_type(t));
        EXPECT_FALSE(is_const_type(t));
        EXPECT_EQ(bytes_of_type(t), 4);
    }
    {
        pType t = make_basic_type(IMM_F64, true);
        EXPECT_FALSE(is_integer(t));
        EXPECT_TRUE(is_float(t));
        EXPECT_TRUE(is_signed_type(t));
        EXPECT_FALSE(is_array(t));
        EXPECT_FALSE(is_struct(t));
        EXPECT_FALSE(is_pointer(t));
        EXPECT_TRUE(is_basic_type(t));
        EXPECT_TRUE(is_const_type(t));
        EXPECT_EQ(bytes_of_type(t), 8);
    }
    {
        pType t = make_array_type(make_basic_type(IMM_F64, false), 10);
        EXPECT_FALSE(is_integer(t));
        EXPECT_FALSE(is_float(t));
        EXPECT_FALSE(is_signed_type(t));
        EXPECT_TRUE(is_array(t));
        EXPECT_FALSE(is_struct(t));
        EXPECT_FALSE(is_pointer(t));
        EXPECT_FALSE(is_basic_type(t));
        EXPECT_FALSE(is_const_type(t));
        EXPECT_EQ(bytes_of_type(t), 8 * 10);
    }
}

TEST(test_type, type_convertor) {
    pType elem_type = make_basic_type(IMM_I32, false);
    EXPECT_TRUE(is_same_type(elem_type, to_pointed_type(make_pointer_type(elem_type, false))));
    EXPECT_TRUE(is_same_type(elem_type, to_elem_type(make_array_type(elem_type, 10))));
}
