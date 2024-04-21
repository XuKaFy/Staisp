#include "read_file.h"

#include "staisp_parser.h"
#include "convert_ast_to_ir.h"

#include "opt.h"

#define TEST_A_FILE(x, y) \
TEST(test_code_##x, code_##y) { \
    pCode p_code = read_test_file("tests/" #x "/" #y ".sta"); \
    AstProg root = Staisp::Parser().parse(p_code); \
    Ir::pModule mod = AstToIr::Convertor().generate(root); \
    std::ofstream out; \
    out.open("test.ll", std::fstream::out); \
    out << mod->print_module(); \
    out.close(); \
    EXPECT_EQ(system("lli test.ll"), 0); \
    Optimize::optimize(mod); \
    out.open("test.opt.ll", std::fstream::out); \
    out << mod->print_module(); \
    out.close(); \
    EXPECT_EQ(system("lli test.opt.ll"), 0); \
}

TEST_A_FILE(algorithms, fib)
TEST_A_FILE(algorithms, gcd)
TEST_A_FILE(algorithms, sort)

TEST_A_FILE(array, arr)
TEST_A_FILE(array, arr2)
TEST_A_FILE(array, arr3)

TEST_A_FILE(basic_branch, break_in_block)
TEST_A_FILE(basic_branch, brk_ctn)

TEST_A_FILE(basic_env, env)

TEST_A_FILE(basic_type, diff_type_calc)
TEST_A_FILE(basic_type, float)

// TEST_A_FILE(pointer, ptr)
