// THIS FILE IS MACHINE GENERATED, DO NOT EDIT BY HAND
#include <gtest/gtest.h>
#include <string>

void test_sysy_frontend(const std::string &filename);
void test_sysy_backend(const std::string &filename);

void run_sysy(const std::string &filename) {
    test_sysy_frontend(filename);
    test_sysy_backend(filename);
}

GTEST_TEST(test_sysy_functional, 00_main) {                 
    run_sysy("testdata/functional/00_main");
}

GTEST_TEST(test_sysy_functional, 01_var_defn2) {                 
    run_sysy("testdata/functional/01_var_defn2");
}

GTEST_TEST(test_sysy_functional, 02_var_defn3) {                 
    run_sysy("testdata/functional/02_var_defn3");
}

GTEST_TEST(test_sysy_functional, 03_arr_defn2) {                 
    run_sysy("testdata/functional/03_arr_defn2");
}

GTEST_TEST(test_sysy_functional, 04_arr_defn3) {                 
    run_sysy("testdata/functional/04_arr_defn3");
}

GTEST_TEST(test_sysy_functional, 05_arr_defn4) {                 
    run_sysy("testdata/functional/05_arr_defn4");
}

GTEST_TEST(test_sysy_functional, 06_const_var_defn2) {                 
    run_sysy("testdata/functional/06_const_var_defn2");
}

GTEST_TEST(test_sysy_functional, 07_const_var_defn3) {                 
    run_sysy("testdata/functional/07_const_var_defn3");
}

GTEST_TEST(test_sysy_functional, 08_const_array_defn) {                 
    run_sysy("testdata/functional/08_const_array_defn");
}

GTEST_TEST(test_sysy_functional, 09_func_defn) {                 
    run_sysy("testdata/functional/09_func_defn");
}

GTEST_TEST(test_sysy_functional, 10_var_defn_func) {                 
    run_sysy("testdata/functional/10_var_defn_func");
}

GTEST_TEST(test_sysy_functional, 11_add2) {                 
    run_sysy("testdata/functional/11_add2");
}

GTEST_TEST(test_sysy_functional, 12_addc) {                 
    run_sysy("testdata/functional/12_addc");
}

GTEST_TEST(test_sysy_functional, 13_sub2) {                 
    run_sysy("testdata/functional/13_sub2");
}

GTEST_TEST(test_sysy_functional, 14_subc) {                 
    run_sysy("testdata/functional/14_subc");
}

GTEST_TEST(test_sysy_functional, 15_mul) {                 
    run_sysy("testdata/functional/15_mul");
}

GTEST_TEST(test_sysy_functional, 16_mulc) {                 
    run_sysy("testdata/functional/16_mulc");
}

GTEST_TEST(test_sysy_functional, 17_div) {                 
    run_sysy("testdata/functional/17_div");
}

GTEST_TEST(test_sysy_functional, 18_divc) {                 
    run_sysy("testdata/functional/18_divc");
}

GTEST_TEST(test_sysy_functional, 19_mod) {                 
    run_sysy("testdata/functional/19_mod");
}

GTEST_TEST(test_sysy_functional, 20_rem) {                 
    run_sysy("testdata/functional/20_rem");
}

GTEST_TEST(test_sysy_functional, 21_if_test2) {                 
    run_sysy("testdata/functional/21_if_test2");
}

GTEST_TEST(test_sysy_functional, 22_if_test3) {                 
    run_sysy("testdata/functional/22_if_test3");
}

GTEST_TEST(test_sysy_functional, 23_if_test4) {                 
    run_sysy("testdata/functional/23_if_test4");
}

GTEST_TEST(test_sysy_functional, 24_if_test5) {                 
    run_sysy("testdata/functional/24_if_test5");
}

GTEST_TEST(test_sysy_functional, 25_while_if) {                 
    run_sysy("testdata/functional/25_while_if");
}

GTEST_TEST(test_sysy_functional, 26_while_test1) {                 
    run_sysy("testdata/functional/26_while_test1");
}

GTEST_TEST(test_sysy_functional, 27_while_test2) {                 
    run_sysy("testdata/functional/27_while_test2");
}

GTEST_TEST(test_sysy_functional, 28_while_test3) {                 
    run_sysy("testdata/functional/28_while_test3");
}

GTEST_TEST(test_sysy_functional, 29_break) {                 
    run_sysy("testdata/functional/29_break");
}

GTEST_TEST(test_sysy_functional, 30_continue) {                 
    run_sysy("testdata/functional/30_continue");
}

GTEST_TEST(test_sysy_functional, 31_while_if_test1) {                 
    run_sysy("testdata/functional/31_while_if_test1");
}

GTEST_TEST(test_sysy_functional, 32_while_if_test2) {                 
    run_sysy("testdata/functional/32_while_if_test2");
}

GTEST_TEST(test_sysy_functional, 33_while_if_test3) {                 
    run_sysy("testdata/functional/33_while_if_test3");
}

GTEST_TEST(test_sysy_functional, 34_arr_expr_len) {                 
    run_sysy("testdata/functional/34_arr_expr_len");
}

GTEST_TEST(test_sysy_functional, 35_op_priority1) {                 
    run_sysy("testdata/functional/35_op_priority1");
}

GTEST_TEST(test_sysy_functional, 36_op_priority2) {                 
    run_sysy("testdata/functional/36_op_priority2");
}

GTEST_TEST(test_sysy_functional, 37_op_priority3) {                 
    run_sysy("testdata/functional/37_op_priority3");
}

GTEST_TEST(test_sysy_functional, 38_op_priority4) {                 
    run_sysy("testdata/functional/38_op_priority4");
}

GTEST_TEST(test_sysy_functional, 39_op_priority5) {                 
    run_sysy("testdata/functional/39_op_priority5");
}

GTEST_TEST(test_sysy_functional, 40_unary_op) {                 
    run_sysy("testdata/functional/40_unary_op");
}

GTEST_TEST(test_sysy_functional, 41_unary_op2) {                 
    run_sysy("testdata/functional/41_unary_op2");
}

GTEST_TEST(test_sysy_functional, 42_empty_stmt) {                 
    run_sysy("testdata/functional/42_empty_stmt");
}

GTEST_TEST(test_sysy_functional, 43_logi_assign) {                 
    run_sysy("testdata/functional/43_logi_assign");
}

GTEST_TEST(test_sysy_functional, 44_stmt_expr) {                 
    run_sysy("testdata/functional/44_stmt_expr");
}

GTEST_TEST(test_sysy_functional, 45_comment1) {                 
    run_sysy("testdata/functional/45_comment1");
}

GTEST_TEST(test_sysy_functional, 46_hex_defn) {                 
    run_sysy("testdata/functional/46_hex_defn");
}

GTEST_TEST(test_sysy_functional, 47_hex_oct_add) {                 
    run_sysy("testdata/functional/47_hex_oct_add");
}

GTEST_TEST(test_sysy_functional, 48_assign_complex_expr) {                 
    run_sysy("testdata/functional/48_assign_complex_expr");
}

GTEST_TEST(test_sysy_functional, 49_if_complex_expr) {                 
    run_sysy("testdata/functional/49_if_complex_expr");
}

GTEST_TEST(test_sysy_functional, 50_short_circuit) {                 
    run_sysy("testdata/functional/50_short_circuit");
}

GTEST_TEST(test_sysy_functional, 51_short_circuit3) {                 
    run_sysy("testdata/functional/51_short_circuit3");
}

GTEST_TEST(test_sysy_functional, 52_scope) {                 
    run_sysy("testdata/functional/52_scope");
}

GTEST_TEST(test_sysy_functional, 53_scope2) {                 
    run_sysy("testdata/functional/53_scope2");
}

GTEST_TEST(test_sysy_functional, 54_hidden_var) {                 
    run_sysy("testdata/functional/54_hidden_var");
}

GTEST_TEST(test_sysy_functional, 55_sort_test1) {                 
    run_sysy("testdata/functional/55_sort_test1");
}

GTEST_TEST(test_sysy_functional, 56_sort_test2) {                 
    run_sysy("testdata/functional/56_sort_test2");
}

GTEST_TEST(test_sysy_functional, 57_sort_test3) {                 
    run_sysy("testdata/functional/57_sort_test3");
}

GTEST_TEST(test_sysy_functional, 58_sort_test4) {                 
    run_sysy("testdata/functional/58_sort_test4");
}

GTEST_TEST(test_sysy_functional, 59_sort_test5) {                 
    run_sysy("testdata/functional/59_sort_test5");
}

GTEST_TEST(test_sysy_functional, 60_sort_test6) {                 
    run_sysy("testdata/functional/60_sort_test6");
}

GTEST_TEST(test_sysy_functional, 61_sort_test7) {                 
    run_sysy("testdata/functional/61_sort_test7");
}

GTEST_TEST(test_sysy_functional, 62_percolation) {                 
    run_sysy("testdata/functional/62_percolation");
}

GTEST_TEST(test_sysy_functional, 63_big_int_mul) {                 
    run_sysy("testdata/functional/63_big_int_mul");
}

GTEST_TEST(test_sysy_functional, 64_calculator) {                 
    run_sysy("testdata/functional/64_calculator");
}

GTEST_TEST(test_sysy_functional, 65_color) {                 
    run_sysy("testdata/functional/65_color");
}

GTEST_TEST(test_sysy_functional, 66_exgcd) {                 
    run_sysy("testdata/functional/66_exgcd");
}

GTEST_TEST(test_sysy_functional, 67_reverse_output) {                 
    run_sysy("testdata/functional/67_reverse_output");
}

GTEST_TEST(test_sysy_functional, 68_brainfk) {                 
    run_sysy("testdata/functional/68_brainfk");
}

GTEST_TEST(test_sysy_functional, 69_expr_eval) {                 
    run_sysy("testdata/functional/69_expr_eval");
}

GTEST_TEST(test_sysy_functional, 70_dijkstra) {                 
    run_sysy("testdata/functional/70_dijkstra");
}

GTEST_TEST(test_sysy_functional, 71_full_conn) {                 
    run_sysy("testdata/functional/71_full_conn");
}

GTEST_TEST(test_sysy_functional, 72_hanoi) {                 
    run_sysy("testdata/functional/72_hanoi");
}

GTEST_TEST(test_sysy_functional, 73_int_io) {                 
    run_sysy("testdata/functional/73_int_io");
}

GTEST_TEST(test_sysy_functional, 74_kmp) {                 
    run_sysy("testdata/functional/74_kmp");
}

GTEST_TEST(test_sysy_functional, 75_max_flow) {                 
    run_sysy("testdata/functional/75_max_flow");
}

GTEST_TEST(test_sysy_functional, 76_n_queens) {                 
    run_sysy("testdata/functional/76_n_queens");
}

GTEST_TEST(test_sysy_functional, 77_substr) {                 
    run_sysy("testdata/functional/77_substr");
}

GTEST_TEST(test_sysy_functional, 78_side_effect) {                 
    run_sysy("testdata/functional/78_side_effect");
}

GTEST_TEST(test_sysy_functional, 79_var_name) {                 
    run_sysy("testdata/functional/79_var_name");
}

GTEST_TEST(test_sysy_functional, 80_chaos_token) {                 
    run_sysy("testdata/functional/80_chaos_token");
}

GTEST_TEST(test_sysy_functional, 81_skip_spaces) {                 
    run_sysy("testdata/functional/81_skip_spaces");
}

GTEST_TEST(test_sysy_functional, 82_long_func) {                 
    run_sysy("testdata/functional/82_long_func");
}

GTEST_TEST(test_sysy_functional, 83_long_array) {                 
    run_sysy("testdata/functional/83_long_array");
}

GTEST_TEST(test_sysy_functional, 84_long_array2) {                 
    run_sysy("testdata/functional/84_long_array2");
}

GTEST_TEST(test_sysy_functional, 85_long_code) {                 
    run_sysy("testdata/functional/85_long_code");
}

GTEST_TEST(test_sysy_functional, 86_long_code2) {                 
    run_sysy("testdata/functional/86_long_code2");
}

GTEST_TEST(test_sysy_functional, 87_many_params) {                 
    run_sysy("testdata/functional/87_many_params");
}

GTEST_TEST(test_sysy_functional, 88_many_params2) {                 
    run_sysy("testdata/functional/88_many_params2");
}

GTEST_TEST(test_sysy_functional, 89_many_globals) {                 
    run_sysy("testdata/functional/89_many_globals");
}

GTEST_TEST(test_sysy_functional, 90_many_locals) {                 
    run_sysy("testdata/functional/90_many_locals");
}

GTEST_TEST(test_sysy_functional, 91_many_locals2) {                 
    run_sysy("testdata/functional/91_many_locals2");
}

GTEST_TEST(test_sysy_functional, 92_register_alloc) {                 
    run_sysy("testdata/functional/92_register_alloc");
}

GTEST_TEST(test_sysy_functional, 93_nested_calls) {                 
    run_sysy("testdata/functional/93_nested_calls");
}

GTEST_TEST(test_sysy_functional, 94_nested_loops) {                 
    run_sysy("testdata/functional/94_nested_loops");
}

GTEST_TEST(test_sysy_functional, 95_float) {                 
    run_sysy("testdata/functional/95_float");
}

GTEST_TEST(test_sysy_functional, 96_matrix_add) {                 
    run_sysy("testdata/functional/96_matrix_add");
}

GTEST_TEST(test_sysy_functional, 97_matrix_sub) {                 
    run_sysy("testdata/functional/97_matrix_sub");
}

GTEST_TEST(test_sysy_functional, 98_matrix_mul) {                 
    run_sysy("testdata/functional/98_matrix_mul");
}

GTEST_TEST(test_sysy_functional, 99_matrix_tran) {                 
    run_sysy("testdata/functional/99_matrix_tran");
}

GTEST_TEST(test_sysy_h_functional, 00_comment2) {                 
    run_sysy("testdata/h_functional/00_comment2");
}

GTEST_TEST(test_sysy_h_functional, 01_multiple_returns) {                 
    run_sysy("testdata/h_functional/01_multiple_returns");
}

GTEST_TEST(test_sysy_h_functional, 02_ret_in_block) {                 
    run_sysy("testdata/h_functional/02_ret_in_block");
}

GTEST_TEST(test_sysy_h_functional, 03_branch) {                 
    run_sysy("testdata/h_functional/03_branch");
}

GTEST_TEST(test_sysy_h_functional, 04_break_continue) {                 
    run_sysy("testdata/h_functional/04_break_continue");
}

GTEST_TEST(test_sysy_h_functional, 05_param_name) {                 
    run_sysy("testdata/h_functional/05_param_name");
}

GTEST_TEST(test_sysy_h_functional, 06_func_name) {                 
    run_sysy("testdata/h_functional/06_func_name");
}

GTEST_TEST(test_sysy_h_functional, 07_arr_init_nd) {                 
    run_sysy("testdata/h_functional/07_arr_init_nd");
}

GTEST_TEST(test_sysy_h_functional, 08_global_arr_init) {                 
    run_sysy("testdata/h_functional/08_global_arr_init");
}

GTEST_TEST(test_sysy_h_functional, 09_BFS) {                 
    run_sysy("testdata/h_functional/09_BFS");
}

GTEST_TEST(test_sysy_h_functional, 10_DFS) {                 
    run_sysy("testdata/h_functional/10_DFS");
}

GTEST_TEST(test_sysy_h_functional, 11_BST) {                 
    run_sysy("testdata/h_functional/11_BST");
}

GTEST_TEST(test_sysy_h_functional, 12_DSU) {                 
    run_sysy("testdata/h_functional/12_DSU");
}

GTEST_TEST(test_sysy_h_functional, 13_LCA) {                 
    run_sysy("testdata/h_functional/13_LCA");
}

GTEST_TEST(test_sysy_h_functional, 14_dp) {                 
    run_sysy("testdata/h_functional/14_dp");
}

GTEST_TEST(test_sysy_h_functional, 15_graph_coloring) {                 
    run_sysy("testdata/h_functional/15_graph_coloring");
}

GTEST_TEST(test_sysy_h_functional, 16_k_smallest) {                 
    run_sysy("testdata/h_functional/16_k_smallest");
}

GTEST_TEST(test_sysy_h_functional, 17_maximal_clique) {                 
    run_sysy("testdata/h_functional/17_maximal_clique");
}

GTEST_TEST(test_sysy_h_functional, 18_prim) {                 
    run_sysy("testdata/h_functional/18_prim");
}

GTEST_TEST(test_sysy_h_functional, 19_search) {                 
    run_sysy("testdata/h_functional/19_search");
}

GTEST_TEST(test_sysy_h_functional, 20_sort) {                 
    run_sysy("testdata/h_functional/20_sort");
}

GTEST_TEST(test_sysy_h_functional, 21_union_find) {                 
    run_sysy("testdata/h_functional/21_union_find");
}

GTEST_TEST(test_sysy_h_functional, 22_matrix_multiply) {                 
    run_sysy("testdata/h_functional/22_matrix_multiply");
}

GTEST_TEST(test_sysy_h_functional, 23_json) {                 
    run_sysy("testdata/h_functional/23_json");
}

GTEST_TEST(test_sysy_h_functional, 24_array_only) {                 
    run_sysy("testdata/h_functional/24_array_only");
}

GTEST_TEST(test_sysy_h_functional, 25_scope3) {                 
    run_sysy("testdata/h_functional/25_scope3");
}

GTEST_TEST(test_sysy_h_functional, 26_scope4) {                 
    run_sysy("testdata/h_functional/26_scope4");
}

GTEST_TEST(test_sysy_h_functional, 27_scope5) {                 
    run_sysy("testdata/h_functional/27_scope5");
}

GTEST_TEST(test_sysy_h_functional, 28_side_effect2) {                 
    run_sysy("testdata/h_functional/28_side_effect2");
}

GTEST_TEST(test_sysy_h_functional, 29_long_line) {                 
    run_sysy("testdata/h_functional/29_long_line");
}

GTEST_TEST(test_sysy_h_functional, 30_many_dimensions) {                 
    run_sysy("testdata/h_functional/30_many_dimensions");
}

GTEST_TEST(test_sysy_h_functional, 31_many_indirections) {                 
    run_sysy("testdata/h_functional/31_many_indirections");
}

GTEST_TEST(test_sysy_h_functional, 32_many_params3) {                 
    run_sysy("testdata/h_functional/32_many_params3");
}

GTEST_TEST(test_sysy_h_functional, 33_multi_branch) {                 
    run_sysy("testdata/h_functional/33_multi_branch");
}

GTEST_TEST(test_sysy_h_functional, 34_multi_loop) {                 
    run_sysy("testdata/h_functional/34_multi_loop");
}

GTEST_TEST(test_sysy_h_functional, 35_math) {                 
    run_sysy("testdata/h_functional/35_math");
}

GTEST_TEST(test_sysy_h_functional, 36_rotate) {                 
    run_sysy("testdata/h_functional/36_rotate");
}

GTEST_TEST(test_sysy_h_functional, 37_dct) {                 
    run_sysy("testdata/h_functional/37_dct");
}

GTEST_TEST(test_sysy_h_functional, 38_light2d) {                 
    run_sysy("testdata/h_functional/38_light2d");
}

GTEST_TEST(test_sysy_h_functional, 39_fp_params) {                 
    run_sysy("testdata/h_functional/39_fp_params");
}

GTEST_TEST(test_sysy_h_performance, h_1_01) {                 
    run_sysy("testdata/h_performance/h-1-01");
}

GTEST_TEST(test_sysy_h_performance, h_1_02) {                 
    run_sysy("testdata/h_performance/h-1-02");
}

GTEST_TEST(test_sysy_h_performance, h_1_03) {                 
    run_sysy("testdata/h_performance/h-1-03");
}

GTEST_TEST(test_sysy_h_performance, h_2_01) {                 
    run_sysy("testdata/h_performance/h-2-01");
}

GTEST_TEST(test_sysy_h_performance, h_2_02) {                 
    run_sysy("testdata/h_performance/h-2-02");
}

GTEST_TEST(test_sysy_h_performance, h_2_03) {                 
    run_sysy("testdata/h_performance/h-2-03");
}

GTEST_TEST(test_sysy_h_performance, h_3_01) {                 
    run_sysy("testdata/h_performance/h-3-01");
}

GTEST_TEST(test_sysy_h_performance, h_3_02) {                 
    run_sysy("testdata/h_performance/h-3-02");
}

GTEST_TEST(test_sysy_h_performance, h_3_03) {                 
    run_sysy("testdata/h_performance/h-3-03");
}

GTEST_TEST(test_sysy_h_performance, h_4_01) {                 
    run_sysy("testdata/h_performance/h-4-01");
}

GTEST_TEST(test_sysy_h_performance, h_4_02) {                 
    run_sysy("testdata/h_performance/h-4-02");
}

GTEST_TEST(test_sysy_h_performance, h_4_03) {                 
    run_sysy("testdata/h_performance/h-4-03");
}

GTEST_TEST(test_sysy_h_performance, h_5_01) {                 
    run_sysy("testdata/h_performance/h-5-01");
}

GTEST_TEST(test_sysy_h_performance, h_5_02) {                 
    run_sysy("testdata/h_performance/h-5-02");
}

GTEST_TEST(test_sysy_h_performance, h_5_03) {                 
    run_sysy("testdata/h_performance/h-5-03");
}

GTEST_TEST(test_sysy_h_performance, h_6_01) {                 
    run_sysy("testdata/h_performance/h-6-01");
}

GTEST_TEST(test_sysy_h_performance, h_6_02) {                 
    run_sysy("testdata/h_performance/h-6-02");
}

GTEST_TEST(test_sysy_h_performance, h_6_03) {                 
    run_sysy("testdata/h_performance/h-6-03");
}

GTEST_TEST(test_sysy_h_performance, h_7_01) {                 
    run_sysy("testdata/h_performance/h-7-01");
}

GTEST_TEST(test_sysy_h_performance, h_8_01) {                 
    run_sysy("testdata/h_performance/h-8-01");
}

GTEST_TEST(test_sysy_h_performance, h_8_02) {                 
    run_sysy("testdata/h_performance/h-8-02");
}

GTEST_TEST(test_sysy_h_performance, h_8_03) {                 
    run_sysy("testdata/h_performance/h-8-03");
}

GTEST_TEST(test_sysy_performance, 01_mm1) {                 
    run_sysy("testdata/performance/01_mm1");
}

GTEST_TEST(test_sysy_performance, 01_mm2) {                 
    run_sysy("testdata/performance/01_mm2");
}

GTEST_TEST(test_sysy_performance, 01_mm3) {                 
    run_sysy("testdata/performance/01_mm3");
}

GTEST_TEST(test_sysy_performance, 03_sort1) {                 
    run_sysy("testdata/performance/03_sort1");
}

GTEST_TEST(test_sysy_performance, 03_sort2) {                 
    run_sysy("testdata/performance/03_sort2");
}

GTEST_TEST(test_sysy_performance, 03_sort3) {                 
    run_sysy("testdata/performance/03_sort3");
}

GTEST_TEST(test_sysy_performance, 04_spmv1) {                 
    run_sysy("testdata/performance/04_spmv1");
}

GTEST_TEST(test_sysy_performance, 04_spmv2) {                 
    run_sysy("testdata/performance/04_spmv2");
}

GTEST_TEST(test_sysy_performance, 04_spmv3) {                 
    run_sysy("testdata/performance/04_spmv3");
}

GTEST_TEST(test_sysy_performance, fft0) {                 
    run_sysy("testdata/performance/fft0");
}

GTEST_TEST(test_sysy_performance, fft1) {                 
    run_sysy("testdata/performance/fft1");
}

GTEST_TEST(test_sysy_performance, fft2) {                 
    run_sysy("testdata/performance/fft2");
}

GTEST_TEST(test_sysy_performance, gameoflife_gosper) {                 
    run_sysy("testdata/performance/gameoflife-gosper");
}

GTEST_TEST(test_sysy_performance, gameoflife_oscillator) {                 
    run_sysy("testdata/performance/gameoflife-oscillator");
}

GTEST_TEST(test_sysy_performance, gameoflife_p61glidergun) {                 
    run_sysy("testdata/performance/gameoflife-p61glidergun");
}

GTEST_TEST(test_sysy_performance, if_combine1) {                 
    run_sysy("testdata/performance/if-combine1");
}

GTEST_TEST(test_sysy_performance, if_combine2) {                 
    run_sysy("testdata/performance/if-combine2");
}

GTEST_TEST(test_sysy_performance, if_combine3) {                 
    run_sysy("testdata/performance/if-combine3");
}

GTEST_TEST(test_sysy_performance, large_loop_array_1) {                 
    run_sysy("testdata/performance/large_loop_array_1");
}

GTEST_TEST(test_sysy_performance, large_loop_array_2) {                 
    run_sysy("testdata/performance/large_loop_array_2");
}

GTEST_TEST(test_sysy_performance, large_loop_array_3) {                 
    run_sysy("testdata/performance/large_loop_array_3");
}

GTEST_TEST(test_sysy_performance, matmul1) {                 
    run_sysy("testdata/performance/matmul1");
}

GTEST_TEST(test_sysy_performance, matmul2) {                 
    run_sysy("testdata/performance/matmul2");
}

GTEST_TEST(test_sysy_performance, matmul3) {                 
    run_sysy("testdata/performance/matmul3");
}

GTEST_TEST(test_sysy_performance, recursive_call_1) {                 
    run_sysy("testdata/performance/recursive_call_1");
}

GTEST_TEST(test_sysy_performance, recursive_call_2) {                 
    run_sysy("testdata/performance/recursive_call_2");
}

GTEST_TEST(test_sysy_performance, recursive_call_3) {                 
    run_sysy("testdata/performance/recursive_call_3");
}

GTEST_TEST(test_sysy_performance, shuffle0) {                 
    run_sysy("testdata/performance/shuffle0");
}

GTEST_TEST(test_sysy_performance, shuffle1) {                 
    run_sysy("testdata/performance/shuffle1");
}

GTEST_TEST(test_sysy_performance, shuffle2) {                 
    run_sysy("testdata/performance/shuffle2");
}

GTEST_TEST(test_sysy_performance, sl1) {                 
    run_sysy("testdata/performance/sl1");
}

GTEST_TEST(test_sysy_performance, sl2) {                 
    run_sysy("testdata/performance/sl2");
}

GTEST_TEST(test_sysy_performance, sl3) {                 
    run_sysy("testdata/performance/sl3");
}

GTEST_TEST(test_sysy_performance, transpose0) {                 
    run_sysy("testdata/performance/transpose0");
}

GTEST_TEST(test_sysy_performance, transpose1) {                 
    run_sysy("testdata/performance/transpose1");
}

GTEST_TEST(test_sysy_performance, transpose2) {                 
    run_sysy("testdata/performance/transpose2");
}
