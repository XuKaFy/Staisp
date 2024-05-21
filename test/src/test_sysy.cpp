#include "test_framework.h"
#include <gtest/gtest.h>

#define run_sysy_test(s)                                                       \
    TEST(test_sysy, s) {                                                       \
        prepare();                                                             \
        run_sysy("sysy_tests/functional/" #s);                                 \
    }

#define run_sysy_hidden_test(s)                                                \
    TEST(test_hidden_sysy, s) {                                                \
        prepare();                                                             \
        run_sysy("sysy_tests/hidden_functional/" #s);                          \
    }

#define run_sysy_performance_test(s)                                                \
    TEST(test_performance_sysy, s) {                                                \
        prepare();                                                             \
        run_sysy("sysy_tests/performance/" #s);                          \
    }

run_sysy_test(00_main);
run_sysy_test(01_var_defn2);
run_sysy_test(02_var_defn3);
run_sysy_test(03_arr_defn2);
run_sysy_test(04_arr_defn3);
run_sysy_test(05_arr_defn4);
run_sysy_test(06_const_var_defn2);
run_sysy_test(07_const_var_defn3);
run_sysy_test(08_const_array_defn);
run_sysy_test(09_func_defn);
run_sysy_test(10_var_defn_func);
run_sysy_test(11_add2);
run_sysy_test(12_addc);
run_sysy_test(13_sub2);
run_sysy_test(14_subc);
run_sysy_test(15_mul);
run_sysy_test(16_mulc);
run_sysy_test(17_div);
run_sysy_test(18_divc);
run_sysy_test(19_mod);
run_sysy_test(20_rem);
run_sysy_test(21_if_test2);
run_sysy_test(22_if_test3);
run_sysy_test(23_if_test4);
run_sysy_test(24_if_test5);
run_sysy_test(25_while_if);
run_sysy_test(26_while_test1);
run_sysy_test(27_while_test2);
run_sysy_test(28_while_test3);
run_sysy_test(29_break);
run_sysy_test(30_continue);
run_sysy_test(31_while_if_test1);
run_sysy_test(32_while_if_test2);
run_sysy_test(33_while_if_test3);
run_sysy_test(34_arr_expr_len);
run_sysy_test(35_op_priority1);
run_sysy_test(36_op_priority2);
run_sysy_test(37_op_priority3);
run_sysy_test(38_op_priority4);
run_sysy_test(39_op_priority5);
run_sysy_test(40_unary_op);
run_sysy_test(41_unary_op2);
run_sysy_test(42_empty_stmt);
run_sysy_test(43_logi_assign);
run_sysy_test(44_stmt_expr);
run_sysy_test(45_comment1);
run_sysy_test(46_hex_defn);
run_sysy_test(47_hex_oct_add);
run_sysy_test(48_assign_complex_expr);
run_sysy_test(49_if_complex_expr);
run_sysy_test(50_short_circuit);
run_sysy_test(51_short_circuit3);
run_sysy_test(52_scope);
run_sysy_test(53_scope2);
run_sysy_test(54_hidden_var);
run_sysy_test(55_sort_test1);
run_sysy_test(56_sort_test2);
run_sysy_test(57_sort_test3);
run_sysy_test(58_sort_test4);
run_sysy_test(59_sort_test5);
run_sysy_test(60_sort_test6);
run_sysy_test(61_sort_test7);
run_sysy_test(62_percolation);
run_sysy_test(63_big_int_mul);
run_sysy_test(64_calculator);
run_sysy_test(65_color);
run_sysy_test(66_exgcd);
run_sysy_test(67_reverse_output);
run_sysy_test(68_brainfk);
run_sysy_test(69_expr_eval);
run_sysy_test(70_dijkstra);
run_sysy_test(71_full_conn);
run_sysy_test(72_hanoi);
run_sysy_test(73_int_io);
run_sysy_test(74_kmp);
run_sysy_test(75_max_flow);
run_sysy_test(76_n_queens);
run_sysy_test(77_substr);
run_sysy_test(78_side_effect);
run_sysy_test(79_var_name);
run_sysy_test(80_chaos_token);
run_sysy_test(81_skip_spaces);
run_sysy_test(82_long_func);
run_sysy_test(83_long_array);
run_sysy_test(84_long_array2);
run_sysy_test(85_long_code);
run_sysy_test(86_long_code2);
run_sysy_test(87_many_params);
run_sysy_test(88_many_params2);
run_sysy_test(89_many_globals);
run_sysy_test(90_many_locals);
run_sysy_test(91_many_locals2);
run_sysy_test(92_register_alloc);
run_sysy_test(93_nested_calls);
run_sysy_test(94_nested_loops);
run_sysy_test(95_float);
run_sysy_test(96_matrix_add);
run_sysy_test(97_matrix_sub);
run_sysy_test(98_matrix_mul);
run_sysy_test(99_matrix_tran);

run_sysy_hidden_test(00_comment2);
run_sysy_hidden_test(01_multiple_returns);
run_sysy_hidden_test(02_ret_in_block);
run_sysy_hidden_test(03_branch);
run_sysy_hidden_test(04_break_continue);
run_sysy_hidden_test(05_param_name);
run_sysy_hidden_test(06_func_name);
run_sysy_hidden_test(07_arr_init_nd);
run_sysy_hidden_test(08_global_arr_init);
run_sysy_hidden_test(09_BFS);
run_sysy_hidden_test(10_DFS);
run_sysy_hidden_test(11_BST);
run_sysy_hidden_test(12_DSU);
run_sysy_hidden_test(13_LCA);
run_sysy_hidden_test(14_dp);
run_sysy_hidden_test(15_graph_coloring);
run_sysy_hidden_test(16_k_smallest);
run_sysy_hidden_test(17_maximal_clique);
run_sysy_hidden_test(18_prim);
run_sysy_hidden_test(19_search);
run_sysy_hidden_test(20_sort);
run_sysy_hidden_test(21_union_find);
run_sysy_hidden_test(22_matrix_multiply);
run_sysy_hidden_test(23_json);
run_sysy_hidden_test(24_array_only);
run_sysy_hidden_test(25_scope3);
run_sysy_hidden_test(26_scope4);
run_sysy_hidden_test(27_scope5);
run_sysy_hidden_test(28_side_effect2);
run_sysy_hidden_test(29_long_line);
run_sysy_hidden_test(30_many_dimensions);
run_sysy_hidden_test(31_many_indirections);
run_sysy_hidden_test(32_many_params3);
run_sysy_hidden_test(33_multi_branch);
run_sysy_hidden_test(34_multi_loop);
run_sysy_hidden_test(35_math);
run_sysy_hidden_test(36_rotate);
run_sysy_hidden_test(37_dct);
run_sysy_hidden_test(38_light2d);
run_sysy_hidden_test(39_fp_params);

run_sysy_performance_test(00_bitset1);
run_sysy_performance_test(00_bitset2);
run_sysy_performance_test(00_bitset3);
run_sysy_performance_test(01_mm1);
run_sysy_performance_test(01_mm2);
run_sysy_performance_test(01_mm3);
run_sysy_performance_test(02_mv1);
run_sysy_performance_test(02_mv2);
run_sysy_performance_test(02_mv3);
run_sysy_performance_test(03_sort1);
run_sysy_performance_test(04_spmv1);
run_sysy_performance_test(04_spmv2);
run_sysy_performance_test(04_spmv3);
// run_sysy_performance_test(brainfuck-bootstrap);
// run_sysy_performance_test(brainfuck-mandelbrot-nerf);
// run_sysy_performance_test(brainfuck-pi-nerf);
run_sysy_performance_test(conv0);
run_sysy_performance_test(conv1);
run_sysy_performance_test(conv2);
// run_sysy_performance_test(crypto-1);
// run_sysy_performance_test(crypto-2);
// run_sysy_performance_test(crypto-3);
// run_sysy_performance_test(dead-code-elimination-1);
// run_sysy_performance_test(dead-code-elimination-2);
// run_sysy_performance_test(dead-code-elimination-3);
run_sysy_performance_test(fft0);
run_sysy_performance_test(fft1);
run_sysy_performance_test(fft2);
// run_sysy_performance_test(floyd-0);
// run_sysy_performance_test(floyd-1);
// run_sysy_performance_test(floyd-2);
// run_sysy_performance_test(gameoflife-gosper);
// run_sysy_performance_test(gameoflife-oscillator);
// run_sysy_performance_test(gameoflife-p61glidergun);
// run_sysy_performance_test(hoist-1);
// run_sysy_performance_test(hoist-2);
// run_sysy_performance_test(hoist-3);
// run_sysy_performance_test(instruction-combining-1);
// run_sysy_performance_test(instruction-combining-2);
// run_sysy_performance_test(instruction-combining-3);
// run_sysy_performance_test(integer-divide-optimization-1);
// run_sysy_performance_test(integer-divide-optimization-2);
// run_sysy_performance_test(integer-divide-optimization-3);
run_sysy_performance_test(median0);
run_sysy_performance_test(median1);
run_sysy_performance_test(median2);
run_sysy_performance_test(shuffle0);
run_sysy_performance_test(shuffle1);
run_sysy_performance_test(sl1);
run_sysy_performance_test(sl2);
run_sysy_performance_test(sl3);
run_sysy_performance_test(stencil0);
run_sysy_performance_test(stencil1);
run_sysy_performance_test(transpose0);
run_sysy_performance_test(transpose1);
run_sysy_performance_test(transpose2);
